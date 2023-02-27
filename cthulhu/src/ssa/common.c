#include "common.h"

#include "std/str.h"

const char *ssa_opcode_name(ssa_opcode_t op) 
{
#define SSA_OP(id, name) case id: return name;
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
