#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/str.h"
#include "std/vector.h"

#include "report/report.h"

#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/query.h"

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

const ssa_type_t *ssa_get_operand_type(reports_t *reports, const ssa_flow_t *flow, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm: {
        ssa_value_t *value = operand.value;
        return value->type;
    }
    case eOperandReg: {
        const ssa_step_t *step = operand.vreg;
        return step->type;
    }
    case eOperandEmpty: {
        return ssa_type_empty_new("empty");
    }
    case eOperandLocal: {
        CTASSERT(flow != NULL);
        return vector_get(flow->locals, operand.local);
    }
    case eOperandParam: {
        CTASSERT(flow != NULL);
        const ssa_type_t *type = flow->type;
        const ssa_param_t *param = vector_get(type->args, operand.param);
        return param->type;
    }

    default:
        report(reports, eInternal, NULL, "unhandled operand %s", ssa_operand_name(operand.kind));
        return NULL;
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

ssa_type_t *ssa_type_empty_new(const char *name)
{
    return ssa_type_new(eTypeEmpty, name);
}

ssa_type_t *ssa_type_ptr_new(const char *name, const ssa_type_t *type)
{
    ssa_type_t *ptr = ssa_type_new(eTypePointer, name);
    ptr->ptr = type;
    return ptr;
}

ssa_type_t *ssa_type_array_new(const char *name, const ssa_type_t *type, size_t len)
{
    ssa_type_t *array = ssa_type_new(eTypeArray, name);
    array->arr = type;
    array->size = len;
    return array;
}

ssa_value_t *ssa_value_new(const ssa_type_t *type, bool init) 
{
    ssa_value_t *value = ctu_malloc(sizeof(ssa_value_t));
    value->type = type;
    value->initialized = init;
    return value;
}

ssa_value_t *ssa_value_digit_new(const mpz_t digit, const ssa_type_t *type)
{
    ssa_value_t *value = ssa_value_new(type, true);
    memcpy(value->digit, digit, sizeof(mpz_t));
    return value;
}

ssa_value_t *ssa_value_empty_new(const ssa_type_t *type)
{
    ssa_value_t *value = ssa_value_new(type, false);
    return value;
}

ssa_value_t *ssa_value_ptr_new(const ssa_type_t *type, const mpz_t digit)
{
    ssa_value_t *value = ssa_value_new(type, true);
    memcpy(value->digit, digit, sizeof(mpz_t));
    return value;
}

const char *ssa_type_to_string(const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeDigit: return format("digit(%s, %s)", hlir_digit_to_string(type->digit), hlir_sign_to_string(type->sign));
    case eTypeBool: return "bool";
    case eTypePointer: return format("ptr(%s)", ssa_type_to_string(type->ptr));
    case eTypeString: return "string";
    case eTypeArray: return format("array(%s, %zu)", ssa_type_to_string(type->arr), type->size);
    case eTypeStruct: return format("struct(%s)", type->name);
    case eTypeUnion: return format("union(%s)", type->name);
    case eTypeEmpty: return "empty";
    case eTypeUnit: return "unit";

    default: return format("unknown(%s)", ssa_kind_name(type->kind));
    }
}
