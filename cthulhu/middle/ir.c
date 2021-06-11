#include "ir.h"
#include "bison.h"

#include <stdlib.h>
#include <stdio.h>

#include "cthulhu/report/report.h"
#include "cthulhu/util.h"

static unit_t unit_new(size_t size) {
    unit_t unit = { malloc(sizeof(flow_t) * size), size, 0 };
    return unit;
}

static void unit_add(unit_t *unit, flow_t flow) {
    ENSURE_SIZE(unit->flows, unit->len, unit->size, sizeof(flow_t), 4);
    unit->flows[unit->len++] = flow;
}

static flow_t flow_new(size_t size) {
    flow_t flow = { malloc(sizeof(op_t) * size), size, 0 };
    return flow;
}

static size_t flow_add(flow_t *flow, op_t op) {
    ENSURE_SIZE(flow->ops, flow->len, flow->size, sizeof(op_t), 32);
    flow->ops[flow->len] = op;
    return flow->len++;
}

static op_t create_unary(optype_t kind, operand_t expr) {
    op_t op = { kind, { .expr = expr } };
    return op;
}

static op_t create_binary(optype_t kind, operand_t lhs, operand_t rhs) {
    op_t op = { kind, { .lhs = lhs, .rhs = rhs } };
    return op;
}

static operand_t create_vreg(vreg_t vreg) {
    operand_t op = { VREG, { .vreg = vreg } };
    return op;
}

static size_t emit_node(flow_t *flow, node_t *node);

static size_t emit_digit(flow_t *flow, node_t *node) {
    op_t op = { 
        .kind = OP_VALUE, 
        { .expr = { IMM, { .imm = node->digit } } } 
    };
    return flow_add(flow, op);
}

static int get_unary(int op) {
    switch (op) {
    case ADD: return OP_ABS;
    case SUB: return OP_NEG;

    default:
        reportf("get_unary(op = %d)\n", op);
        return OP_EMPTY;
    }
}

static size_t emit_unary(flow_t *flow, node_t *node) {
    op_t op = create_unary(get_unary(node->unary.op),
        create_vreg(emit_node(flow, node->unary.expr))
    );

    return flow_add(flow, op);
}

static int get_binary(int op) {
    switch (op) {
    case ADD: return OP_ADD;
    case SUB: return OP_SUB;
    case DIV: return OP_DIV;
    case MUL: return OP_MUL;
    case REM: return OP_REM;

    default:
        reportf("get_binop(op = %d)\n", op);
        return OP_EMPTY;
    }
}

static size_t emit_binary(flow_t *flow, node_t *node) {
    op_t op = create_binary(get_binary(node->binary.op),
        create_vreg(emit_node(flow, node->binary.lhs)),
        create_vreg(emit_node(flow, node->binary.rhs))
    );

    return flow_add(flow, op);
}


static size_t emit_node(flow_t *flow, node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return emit_digit(flow, node);
    case AST_UNARY: return emit_unary(flow, node);
    case AST_BINARY: return emit_binary(flow, node);

    default:
        reportf("emit_node(node->type = %d)\n", node->type);
        return 0;
    }
}

static flow_t transform_flow(node_t *node) {
    flow_t flow = flow_new(32);
    
    emit_node(&flow, node);

    return flow;
}

unit_t transform_ast(nodes_t *nodes) {
    unit_t unit = unit_new(4);
    for (size_t i = 0; i < nodes->len; i++) {
        flow_t flow = transform_flow(nodes->data + i);
        unit_add(&unit, flow);
    }
    return unit;
}