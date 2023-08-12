#include "common.h"

ssa_operand_t operand_value(ssa_value_t *value)
{
    ssa_operand_t operand = {
        .kind = eOperandImm,
        .value = value
    };
    return operand;
}
