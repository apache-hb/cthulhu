#include "cthulhu/hlir/ops.h"

typedef struct
{
    const char *name;
    const char *symbol;
} operand_name_t;

static const operand_name_t kBinaryOperands[] = {
    [BINARY_ADD] = {"add", "+"},     [BINARY_SUB] = {"sub", "-"},  [BINARY_MUL] = {"mul", "*"},
    [BINARY_DIV] = {"div", "/"},     [BINARY_REM] = {"rem", "%"},

    [BINARY_AND] = {"and", "&&"},    [BINARY_OR] = {"or", "||"},

    [BINARY_SHL] = {"shl", "<<"},    [BINARY_SHR] = {"shr", ">>"}, [BINARY_BITAND] = {"bitand", "&"},
    [BINARY_BITOR] = {"bitor", "|"}, [BINARY_XOR] = {"xor", "^"},
};

static const operand_name_t kCompareOperands[] = {
    [COMPARE_EQ] = {"eq", "=="}, [COMPARE_NEQ] = {"ne", "!="},

    [COMPARE_LT] = {"lt", "<"},  [COMPARE_LTE] = {"le", "<="}, [COMPARE_GT] = {"gt", ">"}, [COMPARE_GTE] = {"ge", ">="},
};

static const operand_name_t kUnaryOperands[] = {
    [UNARY_NEG] = {"neg", "-"},
    [UNARY_ABS] = {"abs", "abs"},

    [UNARY_BITFLIP] = {"bitflip", "~"},
    [UNARY_NOT] = {"not", "!"},
};

// operand accessors

static operand_name_t binary_operand_name(binary_t op)
{
    return kBinaryOperands[op];
}

static operand_name_t compare_operand_name(compare_t op)
{
    return kCompareOperands[op];
}

static operand_name_t unary_operand_name(unary_t op)
{
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
