#include "speed.h"

#include "ctu/util/report.h"

#include <stdlib.h>

static bool convert_to_bool(operand_t op) {
    if (operand_is_bool(op)) {
        return operand_get_bool(op);
    } else {
        mpz_t it;
        operand_get_int(it, op);
        return mpz_cmp_ui(it, 0) != 0;
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

static bool is_zero(mpz_t i) {
    return mpz_cmp_ui(i, 0) == 0;
}

static step_t fold_div(step_t *step, mpz_t lhs, mpz_t rhs, bool *dirty) {
    if (is_zero(rhs)) {
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

    mpz_t it;
    mpz_init(it);
    mpz_cdiv_q(it, lhs, rhs);

    *dirty = true;
    return new_value(step, new_int(it));
}

static step_t fold_rem(step_t *step, mpz_t lhs, mpz_t rhs, bool *dirty) {
    if (is_zero(rhs)) {
        /**
         * same case as divide but for modulo
         */
        report(LEVEL_WARNING, step->source, step->where, "right hand side of modulo evaluates to zero");
        return *step;
    }

    mpz_t it;
    mpz_init(it);
    mpz_mod(it, lhs, rhs);

    *dirty = true;
    return new_value(step, new_int(it));
}

#include <stdio.h>

static step_t fold_math_op(step_t *step, bool *dirty) {
    mpz_t lhs;
    mpz_t rhs;
    mpz_t it;
    mpz_init(it);
    operand_get_int(lhs, step->lhs);
    operand_get_int(rhs, step->rhs);

    bool changed = false;

    switch (step->binary) {
    case BINARY_ADD: 
        changed = true;
        mpz_add(it, lhs, rhs);
        break;
    case BINARY_SUB:
        changed = true;
        mpz_sub(it, lhs, rhs);
        break;
    case BINARY_DIV: 
        return fold_div(step, lhs, rhs, &changed);
    case BINARY_REM: 
        return fold_rem(step, lhs, rhs, &changed);
    case BINARY_MUL: 
        changed = true;
        mpz_mul(it, lhs, rhs);
        break;
    default: 
        assert("invalid math op when folding");
        return *step;
    }

    if (changed) {
        *dirty = true;
        sanitize_range(step->type, it, step->source, step->where);
        return new_value(step, new_int(it));
    } else {
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

    mpz_t it;
    mpz_init(it);
    operand_get_int(it, step->expr);

    if (step->unary == UNARY_ABS) {
        mpz_abs(it, it);
        *step = new_value(step, new_int(it));
        *dirty = true;
    } else if (step->unary == UNARY_NEG) {
        mpz_neg(it, it);
        *step = new_value(step, new_int(it));
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
