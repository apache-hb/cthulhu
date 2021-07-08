#include "ir.h"

#include "ctu/util/report.h"

#include "ctu/sema/sema.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

static where_t NOWHERE = { 0, 0, 0, 0 };

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
        flow->steps = realloc(flow->steps, flow->size * sizeof(step_t));
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

static operand_t get_lvalue(flow_t *flow, node_t *node) {
    if (node->local != NOT_LOCAL) {
        return new_vreg(flow->locals[node->local]);
    } else {
        return new_operand(NONE);
    }
}

static operand_t emit_ref(flow_t *flow, node_t *node) {
    step_t step = new_step(OP_STORE, node->expr);
    step.dst = add_reserve(flow, node->expr);
    step.src = get_lvalue(flow, node->expr);
    add_step(flow, step);
    return step.dst;
}

#include <stdio.h>

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
    operand_t *args = malloc(sizeof(operand_t) * len);

    for (size_t i = 0; i < len; i++) {
        node_t *arg = ast_at(node->args, i);
        printf("kind %d\n", arg->kind);
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
        step_t load = new_step(OP_LOAD, node);
        load.src = local;
        return add_vreg(flow, load);
    }

    /**
     * first try all functions
     */
    for (size_t i = 0; i < num_flows(flow->mod); i++) {
        const char *name = flow->mod->flows[i].name;

        if (strcmp(name, node->ident) == 0) {
            return new_func(i);
        }
    }

    /**
     * now try arguments
     */
    for (size_t i = 0; i < flow->nargs; i++) {
        arg_t arg = flow->args[i];
        if (strcmp(arg.name, arg.name) == 0) {
            return new_arg(i);
        }
    }

    reportf(LEVEL_INTERNAL, node, "unable to resolve %s typechecker let bad code slip", node->ident);

    /* oh no */
    return new_operand(NONE);
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

    step_t *branch = add_step(flow, new_typed_step(OP_BRANCH, NULL));

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

static operand_t emit_var(flow_t *flow, node_t *node) {
    operand_t val = emit_opcode(flow, node->init);

    operand_t out = add_reserve(flow, node);

    step_t step = new_step(OP_STORE, node);
    step.dst = out;
    step.src = val;

    /* store the vreg into the local variable table */
    flow->locals[node->local] = out.vreg;

    return add_vreg(flow, step);
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

    size_t locals = node->locals;

    flow_t flow = { 
        /* name */
        get_decl_name(node), 
        
        /* arguments */
        malloc(sizeof(arg_t) * len), len,

        /* body */
        malloc(sizeof(step_t) * 64), 0, 64, 
        
        /* local variables */
        malloc(sizeof(vreg_t) * locals), locals,

        /* return type */
        get_type(node->result),

        /* parent */
        mod,

        /* exported */
        is_exported(node)
    };

    for (size_t i = 0; i < len; i++) {
        node_t *param = ast_at(params, i);

        arg_t arg = { get_decl_name(param), get_type(param) };

        flow.args[i] = arg;
    }

    emit_opcode(&flow, node->body);

    if (is_void(get_type(node->result))) {
        step_t ret = step_return(VOID_TYPE, new_operand(NONE));
        add_step(&flow, ret);
    }

    return flow;
}

module_t *compile_module(const char *name, nodes_t *nodes) {
    size_t len = ast_len(nodes);
    module_t *mod = malloc(sizeof(module_t));
    mod->name = name;
    mod->flows = malloc(sizeof(flow_t) * len);
    mod->nflows = len;
    
    for (size_t i = 0; i < len; i++) {
        mod->flows[i].name = get_decl_name(ast_at(nodes, i));
    }

    for (size_t i = 0; i < len; i++) {
        mod->flows[i] = compile_flow(mod, ast_at(nodes, i));
    }

    return mod;
}

step_t *step_at(flow_t *flow, size_t idx) {
    return flow->steps + idx;
}

size_t num_flows(module_t *mod) {
    return mod->nflows;
}
