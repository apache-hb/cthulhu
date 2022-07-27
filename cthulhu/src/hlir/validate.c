#include "common.h"

#include "cthulhu/hlir/query.h"

node_t *check_const_expr(reports_t *reports, const hlir_t *expr)
{
    hlir_kind_t kind = get_hlir_kind(expr);
    switch (kind)
    {
    case eHlirBoolLiteral:
    case eHlirDigitLiteral:
    case eHlirStringLiteral:
        return node_invalid();

    case eHlirUnary:
        return check_const_expr(reports, expr->unaryExpr.operand);

    case eHlirCompare:
    case eHlirBinary: {
        node_t *lhs = check_const_expr(reports, expr->lhs);
        node_t *rhs = check_const_expr(reports, expr->rhs);

        if (node_is_valid(lhs))
        {
            return lhs;
        }

        if (node_is_valid(rhs))
        {
            return rhs;
        }

        return node_invalid();
    }

    case eHlirCall:
        return get_hlir_node(expr);

    default:
        report(reports, eInternal, get_hlir_node(expr), "check-const unexpected expression kind %s",
               hlir_kind_to_string(kind));
        return get_hlir_node(expr);
    }
}
