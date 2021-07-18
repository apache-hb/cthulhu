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

static step_t fold_div(step_t *step, int64_t lhs, int64_t rhs, bool *dirty) {
    if (rhs == 0) {
        /**
         * specifically retain the divide opcode
         * when rhs == 0
         * but warn about the divide by zero
         * sometimes people do actually want to cause
         * a floating point exception to test hardware
         * and optimizing that away on purpose 
         * is annoying to the end user
         */
        report(LEVEL_WARNING, step->source, step->where, "right hand side of division evaluates to zero");
        return *step;
    }

    *dirty = true;
    return new_value(step, new_int(lhs / rhs));
}

static step_t fold_rem(step_t *step, int64_t lhs, int64_t rhs, bool *dirty) {
    if (rhs == 0) {
        /**
         * same case as divide but for modulo
         */
        report(LEVEL_WARNING, step->source, step->where, "right hand side of modulo evaluates to zero");
        return *step;
    }

    *dirty = true;
    return new_value(step, new_int(lhs % rhs));
}

static step_t fold_math_op(step_t *step, bool *dirty) {
    int64_t lhs = operand_get_int(step->lhs),
            rhs = operand_get_int(step->rhs);

    switch (step->binary) {
    case BINARY_ADD: 
        *dirty = true;
        return new_value(step, new_int(lhs + rhs));
    case BINARY_SUB: 
        *dirty = true;
        return new_value(step, new_int(lhs - rhs));
    case BINARY_DIV: 
        return fold_div(step, lhs, rhs, dirty);
    case BINARY_REM: 
        return fold_rem(step, lhs, rhs, dirty);
    case BINARY_MUL: 
        *dirty = true;
        return new_value(step, new_int(lhs * rhs));
    default: 
        assert("invalid math op when folding");
        return *step;
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
        *step = fold_math_op(step, dirty);
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

bool fold_consts(flow_t *flow) {
    bool dirty = false;
    for (size_t i = 0; i < flow->len; i++) {
        try_fold(step_at(flow, i), &dirty);
    }
    return dirty;
}
