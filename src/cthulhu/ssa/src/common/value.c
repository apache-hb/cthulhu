// SPDX-License-Identifier: LGPL-3.0-only

#include "arena/arena.h"
#include "common.h"

#include "cthulhu/tree/tree.h"

#include "std/vector.h"

#include "base/panic.h"
#include "memory/memory.h"

#define EXPECT_TYPE(TY, KIND) CTASSERTF(TY->kind == KIND, "expected %s, got %s", ssa_type_name(KIND), ssa_type_name(TY->kind))

static ssa_value_t *ssa_value_new(const ssa_type_t *type, bool init, ssa_value_state_t value)
{
    arena_t *arena = get_global_arena();
    ssa_value_t *self = ARENA_MALLOC(sizeof(ssa_value_t), "ssa_value", NULL, arena);
    self->type = type;
    self->value = value;
    self->init = init;
    return self;
}

ssa_value_t *ssa_value_empty(const ssa_type_t *type)
{
    EXPECT_TYPE(type, eTypeEmpty);
    ssa_literal_value_t literal = { 0 };
    return ssa_value_literal(type, literal);
}

ssa_value_t *ssa_value_unit(const ssa_type_t *type)
{
    EXPECT_TYPE(type, eTypeUnit);
    ssa_literal_value_t literal = { 0 };
    return ssa_value_literal(type, literal);
}

ssa_value_t *ssa_value_bool(const ssa_type_t *type, bool value)
{
    EXPECT_TYPE(type, eTypeBool);
    ssa_literal_value_t literal = { .boolean = value };
    return ssa_value_literal(type, literal);
}

ssa_value_t *ssa_value_digit(const ssa_type_t *type, const mpz_t value)
{
    EXPECT_TYPE(type, eTypeDigit);
    ssa_literal_value_t literal = { 0 };
    mpz_init_set(literal.digit, value);
    return ssa_value_literal(type, literal);
}

ssa_value_t *ssa_value_char(const ssa_type_t *type, char value)
{
    EXPECT_TYPE(type, eTypeDigit);
    ssa_literal_value_t literal = { 0 };
    mpz_init_set_ui(literal.digit, value);
    return ssa_value_literal(type, literal);
}

ssa_value_t *ssa_value_string(const ssa_type_t *type, text_view_t text)
{
    EXPECT_TYPE(type, eTypePointer);

    const char *string = text.text;
    size_t length = text.length;

    ssa_type_pointer_t ptr = type->pointer;
    EXPECT_TYPE(ptr.pointer, eTypeDigit);
    CTASSERTF(ptr.length != 0 && ptr.length != SIZE_MAX, "invalid string length: %zu", ptr.length);

    arena_t *arena = get_global_arena();
    vector_t *data = vector_of(length, arena);
    for (size_t i = 0; i < length; i++)
    {
        ssa_value_t *value = ssa_value_char(ptr.pointer, string[i]);
        vector_set(data, i, value);
    }

    ssa_literal_value_t literal = { .data = data };
    ssa_value_t *self = ssa_value_literal(type, literal);

    return self;
}

ssa_value_t *ssa_value_from(map_t *types, const tree_t *expr)
{
    const ssa_type_t *type = ssa_type_create_cached(types, tree_get_type(expr));
    switch (expr->kind)
    {
    case eTreeExprEmpty: return ssa_value_empty(type);
    case eTreeExprUnit: return ssa_value_unit(type);
    case eTreeExprBool: return ssa_value_bool(type, expr->bool_value);
    case eTreeExprDigit: return ssa_value_digit(type, expr->digit_value);
    case eTreeExprString: return ssa_value_string(type, expr->string_value);
    default: CT_NEVER("Invalid expr kind: %d", expr->kind);
    }
}

ssa_value_t *ssa_value_noinit(const ssa_type_t *type)
{
    return ssa_value_new(type, false, eValueLiteral);
}

ssa_value_t *ssa_value_literal(const ssa_type_t *type, ssa_literal_value_t value)
{
    ssa_value_t *self = ssa_value_new(type, true, eValueLiteral);
    self->literal = value;
    return self;
}

ssa_value_t *ssa_value_relative(const ssa_type_t *type, ssa_relative_value_t value)
{
    ssa_value_t *self = ssa_value_new(type, true, eValueRelative);
    self->relative = value;
    return self;
}

ssa_value_t *ssa_value_opaque_literal(const ssa_type_t *type, mpz_t value)
{
    ssa_literal_value_t literal = { 0 };
    mpz_init_set(literal.pointer, value);
    return ssa_value_literal(type, literal);
}

STA_DECL
ssa_literal_value_t ssa_value_get_literal(const ssa_value_t *value)
{
    CTASSERT(value != NULL);
    CTASSERT(value->value == eValueLiteral);
    return value->literal;
}

STA_DECL
bool ssa_value_get_bool(const ssa_value_t *value)
{
    ssa_literal_value_t literal = ssa_value_get_literal(value);
    CTASSERTF(value->type->kind == eTypeBool, "expected bool, got %s", ssa_type_name(value->type->kind));

    return literal.boolean;
}

STA_DECL
void ssa_value_get_digit(const ssa_value_t *value, mpz_t result)
{
    ssa_literal_value_t literal = ssa_value_get_literal(value);
    CTASSERTF(value->type->kind == eTypeDigit, "expected digit, got %s", ssa_type_name(value->type->kind));

    mpz_init_set(result, literal.digit);
}
