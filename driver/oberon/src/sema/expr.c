#include "oberon/sema/expr.h"

#include "base/panic.h"

static h2_t *sema_digit(h2_t *sema, obr_t *expr, h2_t *implicitType)
{
    // TODO: get correct digit size
    h2_t *type = implicitType != NULL ? implicitType : obr_get_digit_type(eDigitInt, eSignSigned);
    return h2_expr_digit(expr->node, type, expr->digit);
}

static h2_t *sema_unary(h2_t *sema, obr_t *expr, h2_t *implicitType)
{
    h2_t *operand = obr_sema_rvalue(sema, expr->expr, implicitType);
    return h2_expr_unary(expr->node, expr->unary, operand);
}

static h2_t *sema_binary(h2_t *sema, obr_t *expr, h2_t *implicitType)
{
    // TODO: get common type
    h2_t *type = implicitType != NULL ? implicitType : obr_get_digit_type(eDigitInt, eSignSigned);
    h2_t *lhs = obr_sema_rvalue(sema, expr->lhs, implicitType);
    h2_t *rhs = obr_sema_rvalue(sema, expr->rhs, implicitType);

    return h2_expr_binary(expr->node, type, expr->binary, lhs, rhs);
}

static h2_t *sema_compare(h2_t *sema, obr_t *expr, h2_t *implicitType)
{
    // TODO: check types are comparable

    h2_t *lhs = obr_sema_rvalue(sema, expr->lhs, implicitType);
    h2_t *rhs = obr_sema_rvalue(sema, expr->rhs, implicitType);

    return h2_expr_compare(expr->node, obr_get_bool_type(), expr->compare, lhs, rhs);
}

h2_t *obr_sema_rvalue(h2_t *sema, obr_t *expr, h2_t *implicitType)
{
    h2_t *type = implicitType != NULL ? h2_resolve(h2_get_cookie(sema), implicitType) : NULL;

    switch (expr->kind)
    {
    case eObrExprDigit: return sema_digit(sema, expr, type);
    case eObrExprUnary: return sema_unary(sema, expr, type);
    case eObrExprBinary: return sema_binary(sema, expr, type);
    case eObrExprCompare: return sema_compare(sema, expr, type);

    default: NEVER("unknown expr kind %d", expr->kind);
    }
}

///
/// default values
///

h2_t *obr_default_value(const node_t *node, const h2_t *type)
{
    while (h2_is(type, eHlir2Qualify)) { type = h2_get_type(type); }

    switch (h2_get_kind(type))
    {
    case eHlir2TypeBool: return h2_expr_bool(node, type, false);
    case eHlir2TypeUnit: return h2_expr_unit(node, type);

    case eHlir2TypeDigit: {
        mpz_t zero;
        mpz_init(zero);
        return h2_expr_digit(node, type, zero);
    }

    default:
        NEVER("obr-default-value unknown type kind %d", h2_get_kind(type));
    }
}
