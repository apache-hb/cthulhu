#include "common.h"

#include "base/memory.h"
#include "cthulhu/ssa/ssa.h"
#include "std/str.h"
#include "base/panic.h"

#include <string.h>

const char *ssa_opcode_name(ssa_opcode_t op) 
{
#define SSA_OPCODE(id, name) case id: return name;
    switch (op) 
    {
#include "cthulhu/ssa/ssa.inc"
    default: return format("unknown opcode %d", op);
    }
}

const char *ssa_operand_name(ssa_operand_kind_t op) 
{
#define SSA_OPERAND(id, name) case id: return name;
    switch (op) 
    {
#include "cthulhu/ssa/ssa.inc"
    default: return format("unknown operand %d", op);
    }
}

const char *ssa_kind_name(ssa_kind_t kind) 
{
#define SSA_TYPE(id, name) case id: return name;
    switch (kind) 
    {
#include "cthulhu/ssa/ssa.inc"
    default: return format("unknown type %d", kind);
    }
}

ssa_kind_t ssa_get_value_kind(const ssa_value_t *value)
{
    CTASSERT(value != NULL);
    CTASSERT(value->type != NULL);
    return value->type->kind;
}

ssa_param_t *ssa_param_new(const char *name, const ssa_type_t *type)
{
    ssa_param_t *param = ctu_malloc(sizeof(ssa_param_t));
    param->name = name;
    param->type = type;
    return param;
}

ssa_type_t *ssa_type_new(ssa_kind_t kind, const char *name)
{
    ssa_type_t *type = ctu_malloc(sizeof(ssa_type_t));
    type->kind = kind;
    type->name = name;
    return type;
}

ssa_type_t *type_empty_new(const char *name)
{
    return ssa_type_new(eTypeEmpty, name);
}

ssa_value_t *ssa_value_new(const ssa_type_t *type, bool init) 
{
    ssa_value_t *value = ctu_malloc(sizeof(ssa_value_t));
    value->type = type;
    value->initialized = init;
    return value;
}

ssa_value_t *value_digit_new(mpz_t digit, const ssa_type_t *type)
{
    ssa_value_t *value = ssa_value_new(type, true);
    mpz_init_set(value->digit, digit);
    return value;
}

ssa_value_t *value_empty_new(const ssa_type_t *type)
{
    ssa_value_t *value = ssa_value_new(type, false);
    return value;
}

ssa_value_t *value_ptr_new(const ssa_type_t *type, const mpz_t digit)
{
    ssa_value_t *value = ssa_value_new(type, true);
    memcpy(value->digit, digit, sizeof(mpz_t));
    return value;
}
