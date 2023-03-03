#include "common.h"

#include "base/memory.h"
#include "cthulhu/ssa/ssa.h"
#include "std/str.h"
#include "base/panic.h"

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
