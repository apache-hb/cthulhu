#include "oberon/sema/expr.h"

#include "base/panic.h"

static tree_t *sema_digit(tree_t *sema, obr_t *expr, tree_t *implicitType)
{
    // TODO: get correct digit size
    tree_t *type = implicitType != NULL ? implicitType : obr_get_digit_type(eDigitInt, eSignSigned);
    return tree_expr_digit(expr->node, type, expr->digit);
}

static tree_t *sema_unary(tree_t *sema, obr_t *expr, tree_t *implicitType)
{
    tree_t *operand = obr_sema_rvalue(sema, expr->expr, implicitType);
    return tree_expr_unary(expr->node, expr->unary, operand);
}

static tree_t *sema_binary(tree_t *sema, obr_t *expr, tree_t *implicitType)
{
    // TODO: get common type
    tree_t *type = implicitType != NULL ? implicitType : obr_get_digit_type(eDigitInt, eSignSigned);
    tree_t *lhs = obr_sema_rvalue(sema, expr->lhs, implicitType);
    tree_t *rhs = obr_sema_rvalue(sema, expr->rhs, implicitType);

    return tree_expr_binary(expr->node, type, expr->binary, lhs, rhs);
}

static tree_t *sema_compare(tree_t *sema, obr_t *expr, tree_t *implicitType)
{
    // TODO: check types are comparable

    tree_t *lhs = obr_sema_rvalue(sema, expr->lhs, implicitType);
    tree_t *rhs = obr_sema_rvalue(sema, expr->rhs, implicitType);

    return tree_expr_compare(expr->node, obr_get_bool_type(), expr->compare, lhs, rhs);
}

tree_t *obr_sema_rvalue(tree_t *sema, obr_t *expr, tree_t *implicitType)
{
    tree_t *type = implicitType != NULL ? tree_resolve(tree_get_cookie(sema), implicitType) : NULL;

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

tree_t *obr_default_value(const node_t *node, const tree_t *type)
{
    while (tree_is(type, eTreeQualify)) { type = tree_get_type(type); }

    switch (tree_get_kind(type))
    {
    case eTreeTypeBool: return tree_expr_bool(node, type, false);
    case eTreeTypeUnit: return tree_expr_unit(node, type);

    case eTreeTypeDigit: {
        mpz_t zero;
        mpz_init(zero);
        return tree_expr_digit(node, type, zero);
    }

    default:
        NEVER("obr-default-value unknown type kind %d", tree_get_kind(type));
    }
}
