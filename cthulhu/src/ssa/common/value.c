#include "common.h"

#include "cthulhu/tree/tree.h"

#include "std/vector.h"

#include "base/panic.h"
#include "base/memory.h"

#define EXPECT_TYPE(TY, KIND) CTASSERTF(TY->kind == KIND, "expected %s, got %s", ssa_type_name(KIND), ssa_type_name(TY->kind))

ssa_value_t *ssa_value_new(const ssa_type_t *type, bool init)
{
    ssa_value_t *self = ctu_malloc(sizeof(ssa_value_t));
    self->type = type;
    self->init = init;
    return self;
}

ssa_value_t *ssa_value_empty(const ssa_type_t *type)
{
    EXPECT_TYPE(type, eTypeEmpty);
    return ssa_value_new(type, true);
}

ssa_value_t *ssa_value_unit(const ssa_type_t *type)
{
    EXPECT_TYPE(type, eTypeUnit);
    return ssa_value_new(type, true);
}

ssa_value_t *ssa_value_bool(const ssa_type_t *type, bool value)
{
    EXPECT_TYPE(type, eTypeBool);
    ssa_value_t *self = ssa_value_new(type, true);
    self->boolValue = value;
    return self;
}

ssa_value_t *ssa_value_digit(const ssa_type_t *type, const mpz_t value)
{
    EXPECT_TYPE(type, eTypeDigit);
    ssa_value_t *self = ssa_value_new(type, true);
    mpz_init_set(self->digitValue, value);
    return self;
}

ssa_value_t *ssa_value_char(const ssa_type_t *type, char value)
{
    EXPECT_TYPE(type, eTypeDigit);
    ssa_value_t *self = ssa_value_new(type, true);
    mpz_init_set_ui(self->digitValue, value);
    return self;
}

ssa_value_t *ssa_value_string(const ssa_type_t *type, const char *text, size_t length)
{
    EXPECT_TYPE(type, eTypePointer);
    ssa_type_pointer_t ptr = type->pointer;
    EXPECT_TYPE(ptr.pointer, eTypeDigit);
    CTASSERTF(ptr.length != 0 && ptr.length != SIZE_MAX, "invalid string length: %zu", ptr.length);

    ssa_value_t *self = ssa_value_new(type, true);
    self->data = vector_of(length);
    for (size_t i = 0; i < length; i++)
    {
        ssa_value_t *value = ssa_value_char(ptr.pointer, text[i]);
        vector_set(self->data, i, value);
    }
    return self;
}

ssa_value_t *ssa_value_from(const tree_t *expr)
{
    const ssa_type_t *type = ssa_type_from(expr->type);
    switch (expr->kind)
    {
    case eTreeExprEmpty: return ssa_value_empty(type);
    case eTreeExprUnit: return ssa_value_unit(type);
    case eTreeExprBool: return ssa_value_bool(type, expr->boolValue);
    case eTreeExprDigit: return ssa_value_digit(type, expr->digitValue);
    case eTreeExprString: return ssa_value_string(type, expr->stringValue, expr->stringLength);
    default: NEVER("Invalid expr kind: %d", expr->kind);
    }
}

ssa_value_t *ssa_value_noinit(const ssa_type_t *type)
{
    return ssa_value_new(type, false);
}
