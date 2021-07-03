#include "speed.h"

#include "ctu/util/report.h"

#include <stdlib.h>

static bool convert_to_bool(operand_t op) {
    if (operand_is_bool(op)) {
        return operand_get_bool(op);
    } else {
        return operand_get_int(op) != 0;
    }
}

static void fold_cast(step_t *step, bool *dirty) {
    type_t *to = step_type(step);
    if (is_boolean(to) && operand_is_imm(step->value)) {
        *dirty = true;
        *step = new_value(step,
            new_bool(convert_to_bool(step->value))
        );
    }
}

static int64_t fold_math_op(step_t *step) {
    int64_t lhs = operand_get_int(step->lhs),
            rhs = operand_get_int(step->rhs);

    switch (step->binary) {
    case BINARY_ADD: return lhs + rhs;
    case BINARY_SUB: return lhs - rhs;
    case BINARY_DIV: return lhs / rhs;
    case BINARY_MUL: return lhs * rhs;
    case BINARY_REM: return lhs % rhs;
    default: 
        assert("invalid math op when folding");
        return 0;
    }
}

static void fold_binary(step_t *step, bool *dirty) {
    /**
     * if we dont know both operands then we cant fold
     */
    if (!operand_is_imm(step->lhs) || !operand_is_imm(step->rhs))
        return;

    /**
     * we know both sides will be integers
     */
    if (is_math_op(step->binary)) {
        *step = new_value(step,
            new_int(fold_math_op(step))
        );
        *dirty = true;
    }
}

static void fold_unary(step_t *step, bool *dirty) {
    if (!operand_is_imm(step->expr))
        return;

    if (step->unary == UNARY_ABS) {
        *step = new_value(step, new_int(
            llabs(operand_get_int(step->expr))
        ));
        *dirty = true;
    } else if (step->unary == UNARY_NEG) {
        *step = new_value(step, new_int(
            operand_get_int(step->expr) * -1
        ));
        *dirty = true;
    }
}

static void try_fold(step_t *step, bool *dirty) {
    switch (step->opcode) {
    case OP_CONVERT: fold_cast(step, dirty); break;
    case OP_BINARY: fold_binary(step, dirty); break;
    case OP_UNARY: fold_unary(step, dirty); break;
    default: break;
    }
}

static void fold(flow_t *flow, bool *dirty) {
    for (size_t i = 0; i < flow->len; i++) {
        try_fold(step_at(flow, i), dirty);
    }
}

bool fold_consts(module_t *mod) {
    bool dirty = false;

    for (size_t i = 0; i < num_flows(mod); i++) {
        fold(mod->flows + i, &dirty);
    }

    return dirty; 
}