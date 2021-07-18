#include "ir.h"

#include "ctu/util/report.h"
#include "ctu/debug/ast.h"
#include "ctu/util/util.h"

#include "ctu/sema/sema.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

/**
 * builder functions
 */

static operand_t new_operand(int kind) {
    operand_t op = { kind, { SIZE_MAX } };
    return op;
}

operand_t new_int(int64_t imm) {
    operand_t op = new_operand(IMM);
    op.imm.kind = IMM_INT;
    op.imm.imm_int = imm;
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
    return new_typed_step(op, get_type(node));
}

bool operand_is_imm(operand_t op) {
    return op.kind == IMM;
}

bool operand_is_bool(operand_t op) {
    return operand_is_imm(op) && op.imm.kind == IMM_BOOL;
}

bool operand_is_invalid(operand_t op) {
    return op.kind == NONE;
}

bool operand_get_bool(operand_t op) {
    ASSERT(operand_is_bool(op))("cannot get boolean from a non-boolean immediate");
    return op.imm.imm_bool;
}

int64_t operand_get_int(operand_t op) {
    return op.imm.imm_int;
}

operand_t new_bool(bool b) {
    operand_t op = new_operand(IMM);
    op.imm.kind = IMM_BOOL;
    op.imm.imm_bool = b;
    return op;
}

operand_t new_size(size_t s) {
    operand_t op = new_operand(IMM);
    op.imm.kind = IMM_SIZE;
    op.imm.imm_size = s;
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

static operand_t add_reserve(flow_t *flow, node_t *node) {
    step_t step = new_step(OP_RESERVE, node);
    step.type = set_mut(step.type, true);
    return add_vreg(flow, step);
}

/**
 * codegen logic
 */

static operand_t emit_opcode(flow_t *flow, node_t *node);

static operand_t emit_digit(node_t *node) {
    return new_int(node->digit);
}

static operand_t emit_bool(node_t *node) {
    return new_bool(node->boolean);
}

static bool is_deref(node_t *node) {
    return node->kind == AST_UNARY
        && node->unary == UNARY_DEREF;
}

static operand_t get_global(flow_t *flow, node_t *node) {
    const char *name = get_symbol_name(node);
    for (size_t i = 0; i < num_flows(flow->mod); i++) {
        const char *it = flow->mod->flows[i].name;

        if (it == name) {
            return new_func(i);
        }
    }

    for (size_t i = 0; i < num_vars(flow->mod); i++) {
        const char *it = flow->mod->vars[i].name;

        if (it == name) {
            return new_var(i);
        }
    }

    return new_operand(NONE);
}

static operand_t get_lvalue(flow_t *flow, node_t *node) {
    if (is_deref(node)) {
        step_t step = new_step(OP_LOAD, node->expr);
        step.src = get_lvalue(flow, node->expr);
        return add_vreg(flow, step);
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

    operand_t reserve = add_reserve(flow, node->expr);
    step_t step = new_step(OP_STORE, node->expr);
    step.src = op;
    step.dst = reserve;
    add_step(flow, step);
    return reserve;
}

static step_t emit_deref(node_t *node, operand_t expr) {
    step_t step = new_typed_step(OP_LOAD, get_type(node->expr)->ptr);
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
    
    for (size_t i = 0; i < ast_len(node->stmts); i++) {
        emit_opcode(flow, ast_at(node->stmts, i));
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

    step_t step = step_return(get_type(node), value);

    return add_vreg(flow, step);
}

static operand_t emit_call(flow_t *flow, node_t *node) {
    operand_t expr = emit_opcode(flow, node->expr);

    size_t len = ast_len(node->args);
    operand_t *args = ctu_malloc(sizeof(operand_t) * len);

    for (size_t i = 0; i < len; i++) {
        node_t *arg = ast_at(node->args, i);
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
        if (local.kind == ARG)
            return local;
            
        step_t load = new_step(OP_LOAD, node);
        load.src = local;
        return add_vreg(flow, load);
    }

    /**
     * first try all functions
     */
    local = get_global(flow, node);
    if (operand_is_invalid(local)) {
        return local;
    }

    reportf(LEVEL_INTERNAL, node, "unable to resolve %s typechecker let bad code slip", node->ident);

    /* oh no */
    return new_operand(NONE);
}

static step_t *new_branch(flow_t *flow) {
    return add_step(flow, new_typed_step(OP_BRANCH, NULL));
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

    step_t *branch = new_branch(flow);

    operand_t body = emit_opcode(flow, node->branch);

    operand_t other = node->next == NULL
        ? add_block(flow)
        : emit_branch(flow, node->next);

    branch->cond = cond;
    branch->block = body;
    branch->other = other;

    return head;
}

static operand_t emit_convert(flow_t *flow, node_t *node) {
    operand_t body = emit_opcode(flow, node->expr);

    step_t step = new_step(OP_CONVERT, node);
    step.value = body;

    return add_vreg(flow, step);
}

static void set_local(flow_t *flow, size_t idx, operand_t to) {
    ASSERT(to.kind == VREG)("set_local requires a vreg");

    flow->locals[idx] = to.vreg;
}

static operand_t emit_var(flow_t *flow, node_t *node) {
    
    operand_t val;
    if (node->init) {
        val = emit_opcode(flow, node->init);
    } else {
        val = new_operand(NONE);
    }

    operand_t out = add_reserve(flow, node);

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

static operand_t emit_while(flow_t *flow, node_t *node) {
    operand_t head = add_block(flow);
    operand_t cond = emit_opcode(flow, node->cond);
    
    step_t *branch = new_branch(flow);
    operand_t entry = add_block(flow);

    emit_opcode(flow, node->next);

    add_step(flow, new_jump(head));
    operand_t tail = add_block(flow);

    branch->cond = cond;
    branch->block = entry;
    branch->other = tail;

    return new_operand(NONE);
}

static operand_t emit_opcode(flow_t *flow, node_t *node) {
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
    default:
        reportf(LEVEL_INTERNAL, node, "unknown node kind %d", node->kind);
        return new_operand(NONE);
    }
}



/**
 * external api
 */

static flow_t compile_flow(module_t *mod, node_t *node) {
    ASSERT(node->kind == AST_DECL_FUNC)("compile_flow requires a function");

    nodes_t *params = node->params;
    size_t len = ast_len(params);

    size_t locals = node->locals + len;

    flow_t flow = { 
        /* name */
        get_decl_name(node), 
        
        /* arguments */
        ctu_malloc(sizeof(arg_t) * len), len,

        /* body */
        ctu_malloc(sizeof(step_t) * 64), 0, 64, 
        
        /* local variables */
        ctu_malloc(sizeof(vreg_t) * locals), locals,

        /* return type */
        get_type(node->result),

        /* parent */
        mod,

        /* exported */
        is_exported(node),
        is_used(node)
    };

    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_at(params, i);

        arg_t arg = { get_decl_name(param), get_type(param) };

        operand_t dst = add_reserve(&flow, param);
        step_t step = new_step(OP_STORE, param);
        step.src = new_arg(i);
        step.dst = dst;
        add_step(&flow, step);

        set_local(&flow, param->local, dst);

        flow.args[i] = arg;
    }

    emit_opcode(&flow, node->body);

    if (is_void(get_type(node->result))) {
        step_t ret = step_return(VOID_TYPE, new_operand(NONE));
        add_step(&flow, ret);
    }

    return flow;
}

static var_t compile_var(module_t *mod, node_t *node) {
    ASSERT(node->kind == AST_DECL_VAR)("compile_var requires a variable");

    (void) mod;

    var_t var = { 
        get_decl_name(node), 
        get_type(node),

        is_exported(node),
        is_used(node)
    };

    return var;
}

static size_t count_decls(nodes_t *nodes, ast_t kind) {
    size_t count = 0;
    for (size_t i = 0; i < ast_len(nodes); i++) {
        node_t *node = ast_at(nodes, i);
        if (node->kind == kind) {
            count++;
        }
    }
    return count;
}

module_t *compile_module(const char *name, nodes_t *nodes) {
    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->name = name;
   
    /**
     * TODO: allocate the minimum amount needed
     */
    mod->nflows = count_decls(nodes, AST_DECL_FUNC);
    mod->flows = ctu_malloc(sizeof(flow_t) * mod->nflows);
    
    mod->nvars = count_decls(nodes, AST_DECL_VAR);
    mod->vars = ctu_malloc(sizeof(var_t) * mod->nvars);

    size_t len = ast_len(nodes);

    size_t flow_idx = 0;
    size_t var_idx = 0;
    for (size_t idx = 0; idx < len; idx++) {
        node_t *decl = ast_at(nodes, idx);
        if (decl->kind == AST_DECL_FUNC) {
            mod->flows[flow_idx++].name = get_decl_name(decl);
        } else {
            mod->vars[var_idx++].name = get_decl_name(decl);
        }
    }

    flow_idx = 0;
    var_idx = 0;
    for (size_t i = 0; i < len; i++) {
        node_t *decl = ast_at(nodes, i);
        if (decl->kind == AST_DECL_FUNC) {
            mod->flows[flow_idx++] = compile_flow(mod, decl);
        } else {
            mod->vars[var_idx++] = compile_var(mod, decl);
        }
    }

    return mod;
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
