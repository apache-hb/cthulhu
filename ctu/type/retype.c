#include "retype.h"

static lir_t *select_retype(reports_t *reports, const type_t *type, lir_t *expr);

static lir_t *retype_digit(reports_t *reports, const type_t *type, lir_t *expr) {
    if (mpz_fits_sint_p(expr->digit)) {
        return lir_digit(expr->node, 
            types_common(type, type_digit(SIGNED, TY_INT)),
            expr->digit
        );
    }

    if (mpz_fits_slong_p(expr->digit)) {
        return lir_digit(expr->node,
            types_common(type, type_digit(SIGNED, TY_LONG)),
            expr->digit
        );
    }

    if (mpz_fits_ulong_p(expr->digit)) {
        return lir_digit(expr->node,
            types_common(type, type_digit(UNSIGNED, TY_LONG)),
            expr->digit
        );
    }

    report(reports, ERROR, expr->node, "literal `%s` overflows all native types", mpz_get_str(NULL, 10, expr->digit));
    return expr;
}

static lir_t *retype_unary(reports_t *reports, const type_t *type, lir_t *expr) {
    lir_t *operand = select_retype(reports, type, expr->operand);

    return lir_unary(expr->node,
        types_common(type, lir_type(operand)),
        expr->unary,
        operand
    );
}

static lir_t *retype_logic(reports_t *reports, lir_t *expr) {
    lir_t *lhs = select_retype(reports, type_any(), expr->lhs);
    lir_t *rhs = select_retype(reports, type_any(), expr->rhs);

    return lir_binary(expr->node, type_bool(), expr->binary, lhs, rhs);
}

static lir_t *retype_binary(reports_t *reports, const type_t *type, lir_t *expr) {
    lir_t *lhs = select_retype(reports, type, expr->lhs);
    lir_t *rhs = select_retype(reports, type, expr->rhs);

    switch (expr->binary) {
    case BINARY_EQ: case BINARY_NEQ:
    case BINARY_GT: case BINARY_GTE:
    case BINARY_LT: case BINARY_LTE:
        return retype_logic(reports, expr);

    default:
        return lir_binary(expr->node,
            types_common(type, lir_type(expr)),
            expr->binary, lhs, rhs
        );
    }
}

static lir_t *retype_call(reports_t *reports, const type_t *type, lir_t *expr) {
    const type_t *closure = lir_type(expr->func);

    const type_t *result = types_common(type, closure_result(closure));

    size_t len = vector_len(expr->args);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        lir_t *param = vector_get(expr->args, i);
        lir_t *it = select_retype(reports, param_at(closure, i), param);
        vector_set(args, i, it);
    }

    return lir_call(expr->node, result, expr->func, args);
}

static lir_t *select_retype(reports_t *reports, const type_t *type, lir_t *expr) {
    switch (expr->leaf) {
    case LIR_DIGIT:
        return retype_digit(reports, type, expr);
    case LIR_UNARY:
        return retype_unary(reports, type, expr);
    case LIR_BINARY:
        return retype_binary(reports, type, expr);
    case LIR_CALL:
        return retype_call(reports, type, expr);
    case LIR_PARAM: case LIR_NAME: case LIR_POISON:
    case LIR_BOOL: case LIR_STRING:
        return expr;

    default:
        ctu_assert(reports, "select-retype unknown leaf %d", expr->leaf);
        return expr;
    }
}

lir_t *retype_expr(reports_t *reports, const type_t *type, lir_t *expr) {
    return select_retype(reports, type, expr);
}
