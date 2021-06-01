#include "ir.h"

#include "bison.h"
#include <stdio.h>
#include <stdlib.h>

static unit_t *new_unit(void) {
    unit_t *unit = malloc(sizeof(unit_t));
    unit->size = 8;
    unit->length = 0;
    unit->ops = malloc(sizeof(opcode_t) * unit->size);
    return unit;
}

static size_t unit_add(unit_t *unit, opcode_t op) {
    if (unit->length + 1 > unit->size) {
        unit->size += 8;
        unit->ops = realloc(unit->ops, sizeof(opcode_t) * unit->size);
    }
    unit->ops[unit->length] = op;
    return unit->length++;
}

static size_t build_ir(unit_t *unit, node_t *node);

static operand_t imm(size_t val) {
    operand_t op = { IMM, val };
    return op;
}

static operand_t reg(size_t reg) {
    operand_t op = { REG, reg };
    return op;
}

static size_t build_digit(unit_t *unit, char *text) {
    size_t val = strtoull(text, NULL, 10);
    opcode_t op = { OP_DIGIT, { imm(val) } };
    size_t idx = unit_add(unit, op);

    return idx;
}

static optype_t unary_optype(int op) {
    switch (op) {
    case ADD: return OP_ABS;
    case SUB: return OP_NEG;

    default: 
        fprintf(stderr, "unary_optype(%d)\n", op);
        return -1;
    }
}

static size_t build_unary(unit_t *unit, struct unary_t unary) {
    operand_t body = reg(build_ir(unit, unary.expr));
    opcode_t op = { unary_optype(unary.op), { .expr = body } };
    size_t idx = unit_add(unit, op);

    return idx;
}

static optype_t binary_optype(int op) {
    switch (op) {
    case ADD: return OP_ADD;
    case SUB: return OP_SUB;
    case DIV: return OP_DIV;
    case MUL: return OP_MUL;
    case REM: return OP_REM;

    default:
        fprintf(stderr, "binary_optype(%d)\n", op);
        return -1;
    }
}

static size_t build_binary(unit_t *unit, struct binary_t binary) {
    operand_t lhs = reg(build_ir(unit, binary.lhs)), 
              rhs = reg(build_ir(unit, binary.rhs));
    opcode_t op = { binary_optype(binary.op), { .lhs = lhs, rhs = rhs } };
    size_t idx = unit_add(unit, op);

    return idx;
}

static size_t build_ir(unit_t *unit, node_t *node) {
    switch (node->type) {
    case NODE_DIGIT:
        return build_digit(unit, node->text);
    case NODE_UNARY:
        return build_unary(unit, node->unary);
    case NODE_BINARY:
        return build_binary(unit, node->binary);
    default:
        fprintf(stderr, "build_ir(%d)\n", node->type);
        return 0;
    }
}

unit_t *ir_gen(node_t *node) {
    unit_t *unit = new_unit();

    build_ir(unit, node);

    return unit;
}
