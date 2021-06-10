#include "ir.h"

#include "bison.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define UNIT_SIZE 8

static size_t ir_emit_opcode(unit_t *self, opcode_t op) {
    ENSURE_SIZE(self->data, self->len, self->size, sizeof(opcode_t), 8);
    self->data[self->len] = op;
    return self->len++;
}

opcode_t *ir_opcode_at(unit_t *self, size_t idx) {
    return self->data + idx;
}

static unit_t new_unit(void) {
    unit_t self = { malloc(sizeof(opcode_t) * UNIT_SIZE), UNIT_SIZE, 0 };
    return self;
}

void ir_opcode_apply(opcode_t *op, ir_apply_func_t func, void *data) {
    switch (op->type) {
    case OP_VALUE: case OP_RET: case OP_ABS: case OP_NEG:
        func(op->expr, data);
        break;

    case OP_ADD: case OP_SUB: case OP_DIV: case OP_MUL: case OP_REM:
    case OP_PHI:
        func(op->lhs, data);
        func(op->rhs, data);
        break;

    case OP_JMP:
        func(op->cond, data);
        func(op->label, data);
        break;

    case OP_LABEL: break;

    case OP_CALL: break;
    }
}

static opcode_t ir_opcode(optype_t type) {
    opcode_t op = { type, { } };
    return op;
}

operand_t op_imm(int64_t num) {
    operand_t op = { IMM, { .num = num } };
    return op;
}

static operand_t reg(size_t reg) {
    operand_t op = { REG, { .reg = reg } };
    return op;
}

static size_t ir_emit(unit_t *unit, node_t *node);

static size_t ir_emit_expr(unit_t *unit, optype_t type, operand_t expr) {
    opcode_t op = ir_opcode(type);
    op.expr = expr;
    return ir_emit_opcode(unit, op);
}

static size_t ir_emit_bin(unit_t *unit, optype_t type, operand_t lhs, operand_t rhs) {
    opcode_t op = ir_opcode(type);
    op.lhs = lhs;
    op.rhs = rhs;
    return ir_emit_opcode(unit, op);
}

static size_t ir_emit_digit(unit_t *unit, node_t *node) {
    return ir_emit_expr(unit, OP_VALUE, 
        op_imm(strtoll(node->text, NULL, 10))
    );
}

static optype_t ir_unop(int op) {
    switch (op) {
    case ADD: return OP_ABS;
    case SUB: return OP_NEG;

    default: 
        fprintf(stderr, "ir_unop(%d)\n", op); 
        return INT_MAX;
    }
}

static size_t ir_emit_unary(unit_t *unit, node_t *node) {
    return ir_emit_expr(unit, ir_unop(node->unary.op),
        reg(ir_emit(unit, node->unary.expr))
    );
}

static optype_t ir_binop(int op) {
    switch (op) {
    case ADD: return OP_ADD;
    case SUB: return OP_SUB;
    case DIV: return OP_DIV;
    case MUL: return OP_MUL;
    case REM: return OP_REM;
    default:
        fprintf(stderr, "ir_binop(%d)\n", op);
        return INT_MAX;
    }
}

static size_t ir_emit_binary(unit_t *unit, node_t *node) {
    return ir_emit_bin(unit, ir_binop(node->binary.op),
        reg(ir_emit(unit, node->binary.lhs)),
        reg(ir_emit(unit, node->binary.rhs))
    );
}

static size_t ir_phi(unit_t *unit, size_t lhs, size_t rhs) {
    return ir_emit_bin(unit, OP_PHI, reg(lhs), reg(rhs));
}

static size_t ir_jmp(unit_t *unit, operand_t cond) {
    opcode_t op = ir_opcode(OP_JMP);
    op.cond = cond;
    return ir_emit_opcode(unit, op);
}

static size_t ir_label(unit_t *unit) {
    return ir_emit_opcode(unit, ir_opcode(OP_LABEL));
}

static size_t ir_emit_ternary(unit_t *unit, node_t *node) {
    size_t cond = ir_emit(unit, node->ternary.cond);

    size_t jmp = ir_jmp(unit, reg(cond));

    size_t no = ir_emit(unit, node->ternary.rhs);
    size_t escape = ir_jmp(unit, op_imm(1));

    size_t other = ir_label(unit);

    size_t yes = ir_emit(unit, node->ternary.lhs);

    size_t tail = ir_label(unit);

    ir_opcode_at(unit, escape)->label = reg(tail);
    ir_opcode_at(unit, jmp)->label = reg(other);

    return ir_phi(unit, yes, no);
}

static size_t ir_emit_return(unit_t *unit, node_t *node) {
    return ir_emit_expr(unit, OP_RET, 
        reg(ir_emit(unit, node->expr))
    );
}

static size_t ir_emit(unit_t *unit, node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return ir_emit_digit(unit, node);
    case AST_UNARY: return ir_emit_unary(unit, node);
    case AST_BINARY: return ir_emit_binary(unit, node);
    case AST_TERNARY: return ir_emit_ternary(unit, node);
    case AST_RETURN: return ir_emit_return(unit, node);

    default:
        fprintf(stderr, "ir_emit(node->type = %d)\n", node->type);
        return SIZE_MAX;
    }
}

unit_t ir_emit_node(node_t *node) {
    unit_t unit = new_unit();

    ir_emit(&unit, node);

    return unit;
}
