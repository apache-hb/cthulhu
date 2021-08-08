#include "ir.h"
#include "eval.h"

#include "ctu/util/report.h"
#include "ctu/debug/ast.h"
#include "ctu/debug/ir.h"
#include "ctu/debug/type.h"
#include "ctu/util/util.h"
#include "ctu/util/str.h"

#include "ctu/sema/sema.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

/**
 * builder functions
 */

static operand_t new_operand(int kind) {
    operand_t op = { kind, { SIZE_MAX }, SIZE_MAX };
    return op;
}

operand_t new_int(type_t *type, mpz_t imm) {
    operand_t op = new_operand(IMM);
    op.imm.type = type;
    mpz_init_set(op.imm.num, imm);
    return op;
}

operand_t new_vreg(vreg_t reg) {
    operand_t op = new_operand(VREG);
    op.vreg = reg;
    return op;
}

static operand_t new_func(size_t idx) {
    operand_t op = new_operand(FUNC);
    op.func = idx;
    return op;
}

static operand_t new_var(size_t idx) {
    operand_t op = new_operand(VAR);
    op.var = idx;
    return op;
}

static operand_t new_string(size_t idx) {
    operand_t op = new_operand(STRING);
    op.var = idx;
    return op;
}

static operand_t new_arg(size_t idx) {
    operand_t op = new_operand(ARG);
    op.arg = idx;
    return op;
}

operand_t new_block(size_t label) {
    operand_t op = new_operand(BLOCK);
    op.label = label;
    return op;
}

static step_t new_typed_step(opcode_t op, type_t *type) {
    step_t step = { 
        /* opcode */ op, 
        /* type */ type,
        /* scanner */ NULL,
        /* where */ NOWHERE,
#ifndef _MSC_VER
        /* data */ { }
#endif
    };
    return step;
}

static void node_attach(step_t *step, node_t *node) {
    step->source = node->scanner;
    step->where = node->where;
}

step_t new_jump(operand_t label) {
    step_t step = new_typed_step(OP_JUMP, NULL);
    step.block = label;
    return step;
}

step_t new_value(step_t *old, operand_t value) {
    step_t step = new_typed_step(OP_VALUE, old->type);
    step.value = value;
    return step;
}

type_t *step_type(step_t *step) {
    ASSERT(step->type != NULL)("step did not have a type");
    return step->type;
}

static step_t new_step(opcode_t op, node_t *node) {
    step_t step = new_typed_step(op, get_resolved_type(node));
    step.source = node->scanner;
    step.where = node->where;
    return step;
}

bool operand_is_imm(operand_t op) {
    return op.kind == IMM;
}

bool operand_is_bool(operand_t op) {
    return operand_is_imm(op) 
        && is_boolean(op.imm.type);
}

bool operand_is_invalid(operand_t op) {
    return op.kind == NONE;
}

bool operand_get_bool(operand_t op) {
    ASSERT(operand_is_bool(op))("cannot get boolean from a non-boolean immediate");
    return op.imm.b;
}

void operand_get_int(mpz_t it, operand_t op) {
    mpz_init_set(it, op.imm.num);
}

operand_t new_bool(bool b) {
    operand_t op = new_operand(IMM);
    op.imm.type = BOOL_TYPE;
    op.imm.b = b;
    return op;
}

/**
 * book keeping
 */

static size_t add_step_raw(flow_t *flow, step_t step) {
    if (flow->len + 1 > flow->size) {
        flow->size += 64;
        flow->steps = ctu_realloc(flow->steps, flow->size * sizeof(step_t));
    }

    flow->steps[flow->len] = step;
    return flow->len++;
}

static step_t *add_step(flow_t *flow, step_t step) {
    size_t idx = add_step_raw(flow, step);
    return step_at(flow, idx);
}

static operand_t add_vreg(flow_t *flow, step_t step) {
    return new_vreg(add_step_raw(flow, step));
}

static operand_t add_block(flow_t *flow) {
    /* a block cant have a type so null is fine */
    step_t block = new_typed_step(OP_BLOCK, NULL);
    return new_block(add_step_raw(flow, block));
}

static operand_t add_reserve_type(flow_t *flow, type_t *type, size_t len) {
    step_t step = new_typed_step(OP_RESERVE, type);
    step.type = set_lvalue(type, true);
    step.size = len;
    return add_vreg(flow, step);
}

static operand_t add_reserve(flow_t *flow, node_t *node, size_t len) {
    return add_reserve_type(flow, get_resolved_type(node), len);
}

/**
 * codegen logic
 */

static operand_t emit_opcode(flow_t *flow, node_t *node);

static operand_t emit_digit(node_t *node) {
    return new_int(get_resolved_type(node), node->num);
}

static operand_t emit_bool(node_t *node) {
    return new_bool(node->boolean);
}

static flow_t *get_flow(module_t *mod, size_t idx) {
    return mod->flows + (idx + mod->closures);
}

static operand_t get_global(flow_t *flow, node_t *node) {
    const char *name = list_last(node->ident);
    for (size_t i = 0; i < num_flows(flow->mod); i++) {
        const char *it = get_flow(flow->mod, i)->name;

        if (strcmp(it, name) == 0) {
            return new_func(i);
        }
    }

    for (size_t i = 0; i < num_vars(flow->mod); i++) {
        const char *it = flow->mod->vars[i].name;

        if (strcmp(it, name) == 0) {
            return new_var(i);
        }
    }

    return new_operand(NONE);
}

static size_t field_offset(type_t *type, const char *field) {
    size_t len = type->fields.size;
    for (size_t i = 0; i < len; i++) {
        const char *name = type->fields.fields[i].name;
        if (strcmp(name, field) == 0) {
            return i;
        }
    }

    assert("field offset not found");
    return SIZE_MAX;
}

static void set_local(flow_t *flow, size_t idx, operand_t to) {
    ASSERT(to.kind == VREG)("set_local requires a vreg");
    ASSERT(idx != NOT_LOCAL)("set_local requires a local");

    flow->locals[idx] = to.vreg;
}

static operand_t get_lvalue(flow_t *flow, node_t *node);

static operand_t inner_index(flow_t *flow, node_t *node) {
    operand_t arr = get_lvalue(flow, node->expr);
    operand_t idx = emit_opcode(flow, node->index);

    step_t step = new_step(OP_OFFSET, node->expr);
    step.src = arr;
    step.index = idx;

    operand_t op = add_vreg(flow, step);
    set_local(flow, node->local, op);

    return op;
}

static operand_t get_lvalue(flow_t *flow, node_t *node) {
    if (is_deref(node)) {
        step_t step = new_step(OP_LOAD, node->expr);
        step.src = get_lvalue(flow, node->expr);
        return add_vreg(flow, step);
    }

    if (is_access(node)) {
        node_t *target = node->target;
        operand_t field = get_lvalue(flow, target);
        field.offset = field_offset(get_resolved_type(target), node->field);
        return field;
    }

    if (is_index(node)) {
        return inner_index(flow, node);
    }

    if (node->local == NOT_LOCAL) {
        return get_global(flow, node);
    }

    return new_vreg(flow->locals[node->local]);
}

static operand_t emit_ref(flow_t *flow, node_t *node) {
    operand_t op = get_lvalue(flow, node->expr);
    if (op.kind != ARG)
        return op;

    operand_t reserve = add_reserve(flow, node->expr, 1);
    step_t step = new_step(OP_STORE, node->expr);
    step.src = op;
    step.dst = reserve;
    add_step(flow, step);
    return reserve;
}

static step_t emit_deref(node_t *node, operand_t expr) {
    step_t step = new_typed_step(OP_LOAD, get_resolved_type(node->expr)->ptr);
    step.src = expr;
    return step;
}

static operand_t emit_unary(flow_t *flow, node_t *node) {
    step_t step;

    if (node->unary == UNARY_REF) {
        return emit_ref(flow, node);
    } else if (node->unary == UNARY_DEREF) {
        step = emit_deref(node, emit_opcode(flow, node->expr));
    } else {
        step = new_step(OP_UNARY, node);
        step.unary = node->unary;
        step.expr = emit_opcode(flow, node->expr);
    }

    return add_vreg(flow, step);
}

static operand_t emit_binary(flow_t *flow, node_t *node) {
    operand_t lhs = emit_opcode(flow, node->lhs),
              rhs = emit_opcode(flow, node->rhs);

    step_t step = new_step(OP_BINARY, node);
    step.binary = node->binary;
    step.lhs = lhs;
    step.rhs = rhs;

    node_attach(&step, node);

    return add_vreg(flow, step);
}

static operand_t emit_stmts(flow_t *flow, node_t *node) {
    operand_t block = add_block(flow);
    
    for (size_t i = 0; i < list_len(node->stmts); i++) {
        emit_opcode(flow, list_at(node->stmts, i));
    }

    return block;
}

static step_t step_return(type_t *type, operand_t value) {
    step_t step = new_typed_step(OP_RETURN, type);
    step.value = value;
    return step;
}

static operand_t emit_return(flow_t *flow, node_t *node) {
    operand_t value = node->expr == NULL 
        ? new_operand(NONE)
        : emit_opcode(flow, node->expr);

    step_t step = step_return(get_resolved_type(node), value);

    return add_vreg(flow, step);
}

static operand_t emit_call(flow_t *flow, node_t *node) {
    operand_t expr = emit_opcode(flow, node->expr);

    size_t len = list_len(node->args);
    operand_t *args = ctu_malloc(sizeof(operand_t) * len);

    for (size_t i = 0; i < len; i++) {
        node_t *arg = list_at(node->args, i);
        args[i] = emit_opcode(flow, arg);
    }

    step_t step = new_step(OP_CALL, node);
    step.value = expr;
    step.args = args;
    step.len = len;

    return add_vreg(flow, step);
}

static operand_t emit_symbol(flow_t *flow, node_t *node) {
    operand_t local = get_lvalue(flow, node);
    if (!operand_is_invalid(local)) {
        if (local.kind == ARG || local.kind == FUNC)
            return local;
        
        step_t load = new_step(OP_LOAD, node);
        load.src = local;
        return add_vreg(flow, load);
    }

    reportf(LEVEL_INTERNAL, node, "unable to resolve %s. typechecker let bad code slip", list_last(node->ident));

    /* oh no */
    return new_operand(NONE);
}

static size_t new_branch(flow_t *flow) {
    return add_step_raw(flow, new_typed_step(OP_BRANCH, NULL));
}

static operand_t emit_branch(flow_t *flow, node_t *node) {
    operand_t head = add_block(flow);
    operand_t cond;
    if (node->cond) {
        /* this is an if branch */
        cond = emit_opcode(flow, node->cond);
    } else {
        /* this is an else branch */
        cond = new_bool(true);
    }

    size_t idx = new_branch(flow);

    operand_t body = emit_opcode(flow, node->branch);

    operand_t other = node->next == NULL
        ? add_block(flow)
        : emit_branch(flow, node->next);

    step_t *branch = step_at(flow, idx);

    branch->cond = cond;
    branch->block = body;
    branch->other = other;

    return head;
}

static operand_t emit_cast(flow_t *flow, operand_t value, type_t *type) {
    step_t step = new_typed_step(OP_CONVERT, type);
    step.value = value;

    return add_vreg(flow, step);
}

static operand_t emit_convert(flow_t *flow, node_t *node) {
    operand_t value = emit_opcode(flow, node->expr);

    return emit_cast(flow, value, node->typeof);
}

static operand_t make_var_reserve(flow_t *flow, node_t *node) {
    type_t *ty = get_resolved_type(node);
    if (is_array(ty)) {
        return add_reserve_type(flow, index_type(ty), ty->size);
    } else {
        return add_reserve_type(flow, ty, 1);
    }
}

static operand_t emit_var(flow_t *flow, node_t *node) {
    operand_t val;
    if (node->init) {
        val = emit_opcode(flow, node->init);
    } else {
        val = new_operand(NONE);
    }

    operand_t out = make_var_reserve(flow, node);

    if (!operand_is_invalid(val)) {
        step_t step = new_step(OP_STORE, node);
        step.dst = out;
        step.src = val;
        add_vreg(flow, step);
    }

    /* store the vreg into the local variable table */
    set_local(flow, node->local, out);

    return out;
}

static operand_t emit_assign(flow_t *flow, node_t *node) {
    operand_t dst = get_lvalue(flow, node->dst);
    operand_t src = emit_opcode(flow, node->src);

    step_t step = new_step(OP_STORE, node->dst);
    step.dst = dst;
    step.src = src;
    add_step(flow, step);

    return new_operand(NONE);
}

/**
 * while cond { body }
 * 
 * transforms to
 * 
 * head:
 *   %cond = cond
 *   branch %cond entry else tail
 * entry:
 *   body
 *   jmp head
 * tail:
 */

typedef struct {
    list_t *breaks;
    list_t *continues;
} flow_state_t;

static flow_state_t save_state(flow_t *flow) {
    flow_state_t state = { flow->breaks, flow->continues };

    flow->breaks = new_list(NULL);
    flow->continues = new_list(NULL);
    return state;
}

static void load_state(flow_t *flow, flow_state_t state) {
    flow->breaks = state.breaks;
    flow->continues = state.continues;
}

static void fixup_control_flow(flow_t *flow, operand_t begin, operand_t end) {
    size_t breaks = list_len(flow->breaks);
    size_t continues = list_len(flow->continues);

    /* redirect all breaks to the end of the loop */
    for (size_t i = 0; i < breaks; i++) {
        size_t branch = (size_t)list_at(flow->breaks, i);
        step_at(flow, branch)->block = end;
    }

    /* redirect all continues to the beginning of the loop */
    for (size_t i = 0; i < continues; i++) {
        size_t branch = (size_t)list_at(flow->continues, i);
        step_at(flow, branch)->block = begin;
    }
}

static operand_t emit_while(flow_t *flow, node_t *node) {
    /* save control state */
    flow_state_t state = save_state(flow);

    operand_t head = add_block(flow);
    operand_t cond = emit_opcode(flow, node->cond);
    
    size_t idx = new_branch(flow);
    operand_t entry = add_block(flow);

    emit_opcode(flow, node->next);

    add_step(flow, new_jump(head));
    operand_t tail = add_block(flow);

    step_t *branch = step_at(flow, idx);

    branch->cond = cond;
    branch->block = entry;
    branch->other = tail;

    fixup_control_flow(flow, head, tail);

    /* reload state */
    load_state(flow, state);

    return head;
}

static operand_t emit_string(flow_t *flow, node_t *node) {
    size_t idx = node->local;
    flow->mod->strings[idx] = node->string;
    return new_string(idx); 
}

static operand_t emit_index(flow_t *flow, node_t *node) {
    operand_t src = inner_index(flow, node);

    step_t step = new_step(OP_LOAD, node);
    step.src = src;

    return add_vreg(flow, step);
}

/**
 * we handle break and continue statements 
 * by pushing them onto the respective list
 * and then fixing them up later in the loop statements
 */

static operand_t emit_break(flow_t *flow) {
    size_t idx = add_step_raw(flow, new_jump(new_operand(NONE)));
    list_push(flow->breaks, (void*)idx);
    return new_operand(NONE);
}

static operand_t emit_continue(flow_t *flow) {
    size_t idx = add_step_raw(flow, new_jump(new_operand(NONE)));
    list_push(flow->continues, (void*)idx);
    return new_operand(NONE);
}

static operand_t emit_sizeof(flow_t *flow, node_t *node) {
    flow_t self = { 
        .node = node,
        .name = format("closure%zu", node->local),
        .section = flow->section,
        .args = NULL,
        .nargs = 0,

        .steps = ctu_malloc(sizeof(step_t) * 64), 
        .len = 0, .size = 64, 
        
        .locals = NULL,
        .nlocals = 0,

        .result = size_int(),
        .value = NULL,

        .mod = flow->mod,

        .exported = false,
        .used = true,
        .stub = false,
        .interop = false,

        .breaks = NULL,
        .continues = NULL
    };

    step_t get = new_step(OP_BUILTIN, node);
    get.builtin = BUILTIN_SIZEOF;
    step_t ret = new_step(OP_RETURN, node);

    ret.value = add_vreg(&self, get);
    add_step(&self, ret);

    flow->mod->flows[node->local] = self;

    return new_func(node->local);
}

static operand_t emit_null(void) {
    /* reserve no space, with a type of *void, this is null */
    return new_operand(NIL);
}

static operand_t emit_inner_opcode(flow_t *flow, node_t *node) {
    switch (node->kind) {
    case AST_STMTS: return emit_stmts(flow, node);
    case AST_DIGIT: return emit_digit(node);
    case AST_BOOL: return emit_bool(node);
    case AST_UNARY: return emit_unary(flow, node);
    case AST_BINARY: return emit_binary(flow, node);
    case AST_RETURN: return emit_return(flow, node);
    case AST_CALL: return emit_call(flow, node);
    case AST_SYMBOL: return emit_symbol(flow, node);
    case AST_BRANCH: return emit_branch(flow, node);
    case AST_CAST: return emit_convert(flow, node);
    case AST_DECL_VAR: return emit_var(flow, node);
    case AST_ASSIGN: return emit_assign(flow, node);
    case AST_WHILE: return emit_while(flow, node);
    case AST_STRING: return emit_string(flow, node);
    case AST_INDEX: return emit_index(flow, node);
    case AST_BREAK: return emit_break(flow);
    case AST_CONTINUE: return emit_continue(flow);
    case AST_NULL: return emit_null();
    case AST_BUILTIN_SIZEOF: return emit_sizeof(flow, node);
    case AST_ARG: return emit_inner_opcode(flow, node->arg);

    default:
        reportf(LEVEL_INTERNAL, node, "unknown node kind %d", node->kind);
        return new_operand(NONE);
    }
}

static operand_t emit_opcode(flow_t *flow, node_t *node) {
    operand_t op = emit_inner_opcode(flow, node);
    
    if (node->cast != NULL) {
        op = emit_cast(flow, op, node->cast);
    }

    return op;
}

/**
 * external api
 */

static flow_t compile_flow(module_t *mod, node_t *node) {
    ASSERT(node->kind == AST_DECL_FUNC)("compile_flow requires a function");

    list_t *params = node->params;
    size_t len = list_len(params);

    size_t locals = node->locals + len;

    type_t *result = get_resolved_type(node->result);

    bool stubbed = node->body == NULL;

    flow_t flow = { 
        node,
        /* name */
        get_decl_name(node), 
        node->section,
        
        /* arguments */
        ctu_malloc(sizeof(arg_t) * len), len,

        /* body */
        ctu_malloc(sizeof(step_t) * 64), 0, 64, 
        
        /* local variables */
        ctu_malloc(sizeof(vreg_t) * locals), locals,

        /* return type */
        result,
        NULL,

        /* parent */
        mod,

        /* exported */
        is_exported(node),
        is_used(node),
        stubbed,
        is_interop(node),

        new_list(NULL), /* breaks */
        new_list(NULL) /* continues */
    };

    for (size_t i = 0; i < len; i++) {
        node_t *param = list_at(params, i);

        arg_t arg = { get_decl_name(param), get_resolved_type(param) };

        operand_t dst = add_reserve(&flow, param, 1);
        step_t step = new_step(OP_STORE, param);
        step.src = new_arg(i);
        step.dst = dst;
        add_step(&flow, step);

        set_local(&flow, param->local, dst);

        flow.args[i] = arg;
    }

    if (!stubbed) {
        emit_opcode(&flow, node->body);
    }

    if (is_void(get_resolved_type(node->result))) {
        step_t ret = step_return(VOID_TYPE, new_operand(NONE));
        add_step(&flow, ret);
    }

    return flow;
}

static flow_t compile_var(module_t *mod, node_t *node) {
    ASSERT(node->kind == AST_DECL_VAR)("compile_var requires a variable");

    flow_t self = { 
        .node = node,
        .name = get_decl_name(node),
        .section = node->section,
        .args = NULL,
        .nargs = 0,

        .steps = ctu_malloc(sizeof(step_t) * 64), 
        .len = 0, .size = 64, 
        
        .locals = NULL,
        .nlocals = 0,

        .result = get_resolved_type(node),
        .value = NULL,

        .mod = mod,

        .exported = is_exported(node),
        .used = is_used(node),
        .stub = false,
        .interop = is_interop(node),

        .breaks = NULL,
        .continues = NULL
    };

    if (node->init != NULL) {
        operand_t op = emit_opcode(&self, node->init);
        step_t ret = new_step(OP_RETURN, node->init);
        
        type_t *type = get_resolved_type(node);
        if (!types_equal(type, get_resolved_type(node->init))) {
            step_t cast = new_step(OP_CONVERT, node->type);
            cast.value = op;
            ret.value = add_vreg(&self, cast);
        } else {
            ret.value = op;
        }
        add_step(&self, ret);
    }

    return self;
}

typedef struct {
    size_t idx;
    size_t len;
    type_t **types;
} types_t;

static bool needs_record(types_t *types, type_t *type) {
    if (!is_record(type)) {
        return false;
    }

    for (size_t i = 0; i < types->idx; i++) {
        if (types_equal(type, types->types[i])) {
            return !types->types[i]->emitted;
        }
    }

    return true;
}

static void add_record(types_t *types, type_t *type) {
    if (!is_record(type) || type->emitted) {
        return;
    }

    fields_t fields = type->fields;
    for (size_t i = 0; i < fields.size; i++) {
        field_t field = fields.fields[i];
        if (needs_record(types, field.type)) {
            add_record(types, field.type);
        }
    }

    type->emitted = true;
    types->types[types->idx++] = type;
}

static void reorder_records(module_t *mod) {
    type_t **types = ctu_malloc(sizeof(type_t *) * mod->ntypes);

    types_t ty = { 0, mod->ntypes, types };

    for (size_t i = 0; i < mod->ntypes; i++) {
        add_record(&ty, mod->types[i]);
    }

    mod->types = types;
}

module_t *compile_module(const char *name, unit_t unit) {
    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->name = name;
   
    size_t funcs = list_len(unit.funcs);
    size_t closures = unit.closures;

    mod->closures = closures;

    /* functions are put after closures */
    mod->nflows = funcs + closures;
    mod->flows = ctu_malloc(sizeof(flow_t) * mod->nflows);
    
    mod->nvars = list_len(unit.vars);
    mod->vars = ctu_malloc(sizeof(flow_t) * mod->nvars);

    mod->ntypes = list_len(unit.types);
    mod->types = ctu_malloc(sizeof(type_t*) * mod->ntypes);

    mod->nstrings = unit.strings;
    mod->strings = ctu_malloc(sizeof(char*) * mod->nstrings);

    for (size_t i = 0; i < mod->nstrings; i++) {
        mod->strings[i] = NULL;
    }

    mod->headers = unit.headers;
    mod->libs = unit.libs;

    /**
     * first pass
     */

    for (size_t i = 0; i < funcs; i++) {
        mod->flows[i + closures].name = get_decl_name(list_at(unit.funcs, i));
    }

    for (size_t i = 0; i < mod->nvars; i++) {
        mod->vars[i].name = get_decl_name(list_at(unit.vars, i));
    }

    for (size_t i = 0; i < mod->ntypes; i++) {
        type_t *type = get_resolved_type(list_at(unit.types, i));
        mod->types[i] = type;
    }

    /**
     * second pass 
     */

    for (size_t i = 0; i < funcs; i++) {
        mod->flows[i + closures] = compile_flow(mod, list_at(unit.funcs, i));
    }

    for (size_t i = 0; i < mod->nvars; i++) {
        mod->vars[i] = compile_var(mod, list_at(unit.vars, i));
    }

    /**
     * evaluate global variables
     */

    for (size_t i = 0; i < mod->nvars; i++) {
        flow_t *var = mod->vars + i;
        if (var->node->init) {
            var->value = eval_global(mod, mod->vars + i);
        } else {
            var->value = empty_value();
        }
    }

    reorder_records(mod);

    return mod;
}

const char *flow_name(const flow_t *flow) {
    return flow->name;
}

step_t *step_at(flow_t *flow, size_t idx) {
    return flow->steps + idx;
}

size_t num_flows(module_t *mod) {
    return mod->nflows;
}

size_t num_vars(module_t *mod) {
    return mod->nvars;
}

size_t num_types(module_t *mod) {
    return mod->ntypes;
}

size_t num_strings(module_t *mod) {
    return mod->nstrings;
}
