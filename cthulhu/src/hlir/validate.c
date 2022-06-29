#include "common.h"

#include "cthulhu/hlir/query.h"

node_t check_const_expr(reports_t *reports, const hlir_t *expr)
{
    hlir_kind_t kind = get_hlir_kind(expr);
    switch (kind)
    {
    case HLIR_BOOL_LITERAL:
    case HLIR_DIGIT_LITERAL:
    case HLIR_STRING_LITERAL:
        return node_invalid();

    case HLIR_UNARY:
        return check_const_expr(reports, expr->operand);

    case HLIR_COMPARE:
    case HLIR_BINARY: {
        node_t lhs = check_const_expr(reports, expr->lhs);
        node_t rhs = check_const_expr(reports, expr->rhs);

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

    case HLIR_CALL:
        return get_hlir_node(expr);

    default:
        report(reports, eInternal, get_hlir_node(expr), "check-const unexpected expression kind %s",
               hlir_kind_to_string(kind));
        return get_hlir_node(expr);
    }
}
