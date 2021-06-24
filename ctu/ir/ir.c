#include "ir.h"

#include "ctu/util/report.h"

#include <stdlib.h>

/**
 * builder functions
 */

static operand_t new_operand(int type) {
    operand_t op = { type, { } };
    return op;
}

static operand_t new_imm(uint64_t imm) {
    operand_t op = new_operand(IMM);
    op.imm = imm;
    return op;
}

static operand_t new_vreg(vreg_t reg) {
    operand_t op = new_operand(VREG);
    op.vreg = reg;
    return op;
}

static step_t new_step(opcode_t op, node_t *node) {
    step_t step = { op, node->typeof, { } };
    return step;
}

/**
 * book keeping
 */

static operand_t add_step(flow_t *flow, step_t step) {
    if (flow->len + 1 > flow->size) {
        flow->size += 64;
        flow->steps = realloc(flow->steps, flow->size * sizeof(step_t));
    }

    flow->steps[flow->len] = step;
    return new_vreg(flow->len++);
}

static void add_flow(module_t *mod, flow_t flow) {
    if (mod->len + 1 > mod->size) {
        mod->size += 4;
        mod->flows = realloc(mod->flows, mod->size * sizeof(flow_t));
    }
    mod->flows[mod->len++] = flow;
}

/**
 * codegen logic
 */

static operand_t emit_opcode(flow_t *flow, node_t *node);

static operand_t emit_digit(node_t *node) {
    return new_imm(node->digit);
}

static operand_t emit_unary(flow_t *flow, node_t *node) {
    operand_t expr = emit_opcode(flow, node->expr);

    step_t step = new_step(OP_UNARY, node);
    step.unary = node->unary;
    step.expr = expr;

    return add_step(flow, step);
}

static operand_t emit_binary(flow_t *flow, node_t *node) {
    operand_t lhs = emit_opcode(flow, node->lhs),
              rhs = emit_opcode(flow, node->rhs);

    step_t step = new_step(OP_BINARY, node);
    step.binary = node->binary;
    step.lhs = lhs;
    step.rhs = rhs;

    return add_step(flow, step);
}

static operand_t emit_stmts(flow_t *flow, node_t *node) {
    for (size_t i = 0; i < node->stmts->len; i++) {
        emit_opcode(flow, ast_at(node->stmts, i));
    }
    return new_operand(NONE);
}

static operand_t emit_return(flow_t *flow, node_t *node) {
    operand_t value = emit_opcode(flow, node->expr);

    step_t step = new_step(OP_RETURN, node);
    step.value = value;

    return add_step(flow, step);
}

static operand_t emit_opcode(flow_t *flow, node_t *node) {
    switch (node->kind) {
    case AST_STMTS: return emit_stmts(flow, node);
    case AST_DIGIT: return emit_digit(node);
    case AST_UNARY: return emit_unary(flow, node);
    case AST_BINARY: return emit_binary(flow, node);
    case AST_RETURN: return emit_return(flow, node);
    default:
        reportf(LEVEL_INTERNAL, node, "unknown node kind %d", node->kind);
        return new_operand(NONE);
    }
}



/**
 * external api
 */

static flow_t compile_flow(node_t *node) {
    flow_t flow = { malloc(sizeof(step_t) * 64), 0, 64 };

    ASSERT(node->kind == AST_DECL_FUNC)("compile_flow requires a function");

    emit_opcode(&flow, node->body);

    return flow;
}

module_t compile_module(nodes_t *nodes) {
    module_t mod = { malloc(sizeof(flow_t) * 4), 0, 4 };
    
    for (size_t i = 0; i < nodes->len; i++) {
        flow_t flow = compile_flow(ast_at(nodes, i));
        add_flow(&mod, flow);
    }

    return mod;
}
