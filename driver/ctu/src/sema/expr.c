#include "ctu/sema/expr.h"
#include "ctu/sema/type.h"

#include "ctu/ast.h"

#include "report/report.h"

#include "base/panic.h"

static h2_t *verify_expr_type(h2_t *sema, h2_kind_t kind, const h2_t *type, const char *exprKind, const node_t *node)
{
    if (type == NULL) { return NULL; }
    if (ctu_type_is(type, kind)) { return NULL; }

    report(sema->reports, eFatal, node, "%ss are not implicitly convertable to `%s`", exprKind, ctu_type_string(type));
    return h2_error(node, "invalid implicit type conversion");
}

static h2_t *ctu_sema_bool(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    const h2_t *type = implicitType ? implicitType : ctu_get_bool_type();

    h2_t *verify = verify_expr_type(sema, eHlir2TypeBool, type, "boolean literal", expr->node);
    if (verify != NULL) { return verify; }

    return h2_expr_bool(expr->node, type, expr->boolValue);
}

static h2_t *ctu_sema_int(h2_t *sema, const ctu_t *expr, const h2_t *implicitType)
{
    const h2_t *type = implicitType ? implicitType : ctu_get_int_type(eDigitInt, eSignSigned); // TODO: calculate proper type to use

    h2_t *verify = verify_expr_type(sema, eHlir2TypeDigit, type, "integer literal", expr->node);
    if (verify != NULL) { return verify; }

    return h2_expr_digit(expr->node, type, expr->intValue);
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
    case eCtuExprBool: return ctu_sema_bool(sema, expr, implicitType);
    case eCtuExprInt: return ctu_sema_int(sema, expr, implicitType);

    default: NEVER("invalid rvalue-expr kind %d", expr->kind);
    }
}
