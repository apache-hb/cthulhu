#include "common.h"

#include "cthulhu/hlir/h2.h"

#include "base/panic.h"

ssa_value_t *ssa_value_from(const h2_t *expr)
{
    const ssa_type_t *type = ssa_type_from(expr->type);
    switch (expr->kind)
    {
    case eHlir2ExprEmpty: return ssa_value_empty(type);
    case eHlir2ExprUnit: return ssa_value_unit(type);
    case eHlir2ExprBool: return ssa_value_bool(type, expr->boolValue);
    case eHlir2ExprDigit: return ssa_value_digit(type, expr->digitValue);
    case eHlir2ExprString: return ssa_value_string(type, expr->stringValue, expr->stringLength);
    default: NEVER("Invalid expr kind: %d", expr->kind);
    }
}
