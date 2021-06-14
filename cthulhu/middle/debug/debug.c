#include "debug.h"

#include "cthulhu/util/report.h"

static void debug_operand(operand_t op) {
    switch (op.kind) {
    case VREG: debugf("%%%zu", op.vreg); break;
    case IMM: debugf("%ld", op.imm); break;
    }
}

static void debug_index(size_t idx) {
    debugf("  %%%zu = ", idx);
}

static void debug_index_kind(size_t idx, const char *kind) {
    debugf("  %%%zu = %s ", idx, kind);
}

static void debug_value(size_t idx, op_t *op) {
    debug_index(idx); 
    debug_operand(op->expr);
    debugf("\n");
}

static void debug_unary(size_t idx, const char *kind, op_t *op) {
    debug_index_kind(idx, kind); 
    debug_operand(op->expr);
    debugf("\n");
}

static void debug_binary(size_t idx, const char *kind, op_t *op) {
    debug_index_kind(idx, kind);
    debug_operand(op->lhs);
    debugf(" ");
    debug_operand(op->rhs);
    debugf("\n");
}

static void debug_return(op_t *op) {
    debugf("  ret ");
    debug_operand(op->expr);
    debugf("\n");
}

static void debug_cond(op_t *op) {
    debugf("  jmp .%zu when ", op->block);
    debug_operand(op->cond);
    debugf("\n");
}

static void debug_jmp(op_t *op) {
    debugf("  jmp .%zu\n", op->label);
}

static void debug_branch(branch_t *it) {
    debugf("[ ");
    debug_operand(it->val);
    debugf(" from .%zu ]", it->block);
}

static void debug_phi(size_t idx, op_t *op) {
    debug_index(idx);
    debugf("phi ");
    for (size_t i = 0; i < op->len; i++) {
        if (i) {
            debugf(", ");
        }
        debug_branch(op->branches + i);
    }
    debugf("\n");
}

static void debug_block(size_t idx) {
    debugf(".%zu:\n", idx);
}

static void debug_op(size_t idx, op_t *op) {
    switch (op->kind) {
    case OP_VALUE: debug_value(idx, op); break;

    case OP_ABS: debug_unary(idx, "abs", op); break;
    case OP_NEG: debug_unary(idx, "neg", op); break;

    case OP_ADD: debug_binary(idx, "add", op); break;
    case OP_SUB: debug_binary(idx, "sub", op); break;
    case OP_DIV: debug_binary(idx, "div", op); break;
    case OP_MUL: debug_binary(idx, "mul", op); break;
    case OP_REM: debug_binary(idx, "rem", op); break;

    case OP_RET: debug_return(op); break;

    case OP_COND: debug_cond(op); break;
    case OP_JMP: debug_jmp(op); break;
    case OP_PHI: debug_phi(idx, op); break;
    case OP_BLOCK: debug_block(idx); break;

    default:
        reportf("debug_op(op->kind = %d)\n", op->kind);
        break;
    }
}

void debug_flow(flow_t *flow) {
    debugf("declare %s() {\n", flow->name);
    for (size_t i = 0; i < flow->len; i++) {
        debug_op(i, flow->ops + i);
    }
    debugf("}\n");
}

void debug_unit(unit_t *unit) {
    debugf("module `%s`\n\n", unit->name);
    for (size_t i = 0; i < unit->len; i++) {
        debug_flow(unit->flows + i);
    }
}
