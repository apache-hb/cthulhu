#include "debug.h"

#include "cthulhu/report/report.h"

static void debug_operand(debug_t *debug, operand_t op) {
    switch (op.kind) {
    case VREG: fprintf(debug->out, "%%%zu", op.vreg); break;
    case IMM: fprintf(debug->out, "$%ld", op.imm); break;
    }
}

static void debug_index(debug_t *debug, size_t idx) {
    fprintf(debug->out, "%%%zu = ", idx);
}

static void debug_index_kind(debug_t *debug, size_t idx, const char *kind) {
    fprintf(debug->out, "%%%zu = %s ", idx, kind);
}


static void debug_value(debug_t *debug, size_t idx, op_t *op) {
    debug_index(debug, idx); 
    debug_operand(debug, op->expr);
    fprintf(debug->out, "\n");
}

static void debug_unary(debug_t *debug, size_t idx, const char *kind, op_t *op) {
    debug_index_kind(debug, idx, kind); 
    debug_operand(debug, op->expr);
    fprintf(debug->out, "\n");
}

static void debug_binary(debug_t *debug, size_t idx, const char *kind, op_t *op) {
    debug_index_kind(debug, idx, kind);
    debug_operand(debug, op->lhs);
    fprintf(debug->out, " ");
    debug_operand(debug, op->rhs);
    fprintf(debug->out, "\n");
}

static void debug_op(debug_t *debug, size_t idx, op_t *op) {
    switch (op->kind) {
    case OP_VALUE: debug_value(debug, idx, op); break;

    case OP_ABS: debug_unary(debug, idx, "abs", op); break;
    case OP_NEG: debug_unary(debug, idx, "neg", op); break;

    case OP_ADD: debug_binary(debug, idx, "add", op); break;
    case OP_SUB: debug_binary(debug, idx, "sub", op); break;
    case OP_DIV: debug_binary(debug, idx, "div", op); break;
    case OP_MUL: debug_binary(debug, idx, "mul", op); break;
    case OP_REM: debug_binary(debug, idx, "rem", op); break;

    default:
        reportf("debug_op(op->kind = %d)\n", op->kind);
        break;
    }
}

void debug_flow(debug_t *debug, flow_t *flow) {
    for (size_t i = 0; i < flow->len; i++) {
        debug_op(debug, i, flow->ops + i);
    }
}

void debug_unit(debug_t *debug, unit_t *unit) {
    for (size_t i = 0; i < unit->len; i++) {
        debug_flow(debug, unit->flows + i);
    }
}
