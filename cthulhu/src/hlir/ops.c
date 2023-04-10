#include "cthulhu/hlir/ops.h"

#include "base/panic.h"

typedef struct
{
    const char *name;
    const char *symbol;
} operand_name_t;

static const operand_name_t kBinaryOperands[eBinaryTotal] = {
#define BINARY_OP(ID, NAME, SYMBOL) [ID] = {(NAME), (SYMBOL)},
#include "cthulhu/hlir/hlir-def.inc"
};

static const operand_name_t kCompareOperands[eCompareTotal] = {
#define COMPARE_OP(ID, NAME, SYMBOL) [ID] = {(NAME), (SYMBOL)},
#include "cthulhu/hlir/hlir-def.inc"
};

static const operand_name_t kUnaryOperands[eUnaryTotal] = {
#define UNARY_OP(ID, NAME, SYMBOL) [ID] = {(NAME), (SYMBOL)},
#include "cthulhu/hlir/hlir-def.inc"
};

// operand accessors

static operand_name_t binary_operand_name(binary_t op)
{
    CTASSERT(op < eBinaryTotal);
    return kBinaryOperands[op];
}

static operand_name_t compare_operand_name(compare_t op)
{
    CTASSERT(op < eCompareTotal);
    return kCompareOperands[op];
}

static operand_name_t unary_operand_name(unary_t op)
{
    CTASSERT(op < eUnaryTotal);
    return kUnaryOperands[op];
}

// name accessors

const char *binary_name(binary_t op)
{
    return binary_operand_name(op).name;
}

const char *compare_name(compare_t op)
{
    return compare_operand_name(op).name;
}

const char *unary_name(unary_t op)
{
    return unary_operand_name(op).name;
}

const char *cast_name(cast_t op)
{
#define CAST_OP(ID, NAME) case ID: return NAME;
    switch (op)
    {
#include "cthulhu/hlir/hlir-def.inc"
    default: return "unknown";
    }
}

// symbol accessors

const char *binary_symbol(binary_t op)
{
    return binary_operand_name(op).symbol;
}

const char *compare_symbol(compare_t op)
{
    return compare_operand_name(op).symbol;
}

const char *unary_symbol(unary_t op)
{
    return unary_operand_name(op).symbol;
}
