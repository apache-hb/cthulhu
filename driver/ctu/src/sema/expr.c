#include "ctu/sema/expr.h"

#include "ctu/ast.h"

#include "report/report.h"

#include "base/panic.h"

static h2_t *ctu_sema_int(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    const h2_t *type = implicitType ? implicitType : ctu_get_int_type(eDigitInt, eSignSigned);
    return h2_expr_digit(expr->node, type, expr->intValue);
}

static h2_t *ctu_sema_noinit(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    if (implicitType == NULL)
    {
        report(sema->reports, eFatal, expr->node, "no implicit type in this context for noinit");
        return h2_error(expr->node, "no implicit type");
    }

    return h2_expr_empty(expr->node, implicitType);
}

h2_t *ctu_sema_lvalue(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    NEVER("not implemented");
}

h2_t *ctu_sema_rvalue(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    CTASSERT(expr != NULL);

    switch (expr->kind)
    {
    case eCtuExprInt: return ctu_sema_int(sema, expr, implicitType);
    case eCtuExprNoInit: return ctu_sema_noinit(sema, expr, implicitType);

    default: NEVER("invalid rvalue-expr kind %d", expr->kind);
    }
}
