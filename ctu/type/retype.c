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

static lir_t *retype_binary(reports_t *reports, const type_t *type, lir_t *expr) {
    lir_t *lhs = select_retype(reports, type, expr->lhs);
    lir_t *rhs = select_retype(reports, type, expr->rhs);

    return lir_binary(expr->node,
        types_common(type, lir_type(expr)),
        expr->binary,
        lhs,
        rhs
    );
}

static lir_t *select_retype(reports_t *reports, const type_t *type, lir_t *expr) {
    switch (expr->leaf) {
    case LIR_DIGIT:
        return retype_digit(reports, type, expr);
    case LIR_UNARY:
        return retype_unary(reports, type, expr);
    case LIR_BINARY:
        return retype_binary(reports, type, expr);
    case LIR_NAME:
        return expr;

    default:
        ctu_assert(reports, "select-retype unknown leaf %d", expr->leaf);
        return expr;
    }
}

lir_t *retype_expr(reports_t *reports, const type_t *type, lir_t *expr) {
    return select_retype(reports, type, expr);
}
