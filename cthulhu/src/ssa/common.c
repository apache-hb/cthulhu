#include "common.h"

#include "std/str.h"

const char *ssa_opcode_name(opcode_t op) 
{
#define SSA_OP(id, name) case id: return name;
    switch (op) 
    {
#include "cthulhu/ssa/ssa.inc"
    default: return format("unknown opcode %d", op);
    }
}

const char *ssa_operand_name(opkind_t op) 
{
#define SSA_OPKIND(id, name) case id: return name;
    switch (op) 
    {
#include "cthulhu/ssa/ssa.inc"
    default: return format("unknown operand %d", op);
    }
}
