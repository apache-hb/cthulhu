#include "common.h"

#include "cthulhu/hlir/h2.h"

#include "base/panic.h"
#include "base/memory.h"

ssa_value_t *ssa_value_new(const ssa_type_t *type, bool init)
{
    ssa_value_t *self = ctu_malloc(sizeof(ssa_value_t));
    self->type = type;
    self->init = init;
    return self;
}

ssa_value_t *ssa_value_empty(const ssa_type_t *type)
{
    return ssa_value_new(type, true);
}

ssa_value_t *ssa_value_unit(const ssa_type_t *type)
{
    return ssa_value_new(type, true);
}

ssa_value_t *ssa_value_bool(const ssa_type_t *type, bool value)
{
    ssa_value_t *self = ssa_value_new(type, true);
    self->boolValue = value;
    return self;
}

ssa_value_t *ssa_value_digit(const ssa_type_t *type, const mpz_t value)
{
    ssa_value_t *self = ssa_value_new(type, true);
    mpz_init_set(self->digitValue, value);
    return self;
}

ssa_value_t *ssa_value_string(const ssa_type_t *type, const char *value, size_t length)
{
    ssa_value_t *self = ssa_value_new(type, true);
    self->stringValue = value;
    self->stringLength = length;
    return self;
}

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

ssa_value_t *ssa_value_noinit(const ssa_type_t *type)
{
    return ssa_value_new(type, false);
}
