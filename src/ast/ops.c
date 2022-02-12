#include "cthulhu/ast/ops.h"

static operand_name_t binary_operands[] = {
    [BINARY_ADD] = { "add", "+" },
    [BINARY_SUB] = { "sub", "-" },
    [BINARY_MUL] = { "mul", "*" },
    [BINARY_DIV] = { "div", "/" },
    [BINARY_REM] = { "rem", "%" },

    [BINARY_SHL] = { "shl", "<<" },
    [BINARY_SHR] = { "shr", ">>" },
    [BINARY_AND] = { "bitand", "&" },
    [BINARY_OR] = { "bitor", "|" },
    [BINARY_XOR] = { "xor", "^" }
};

static operand_name_t compare_operands[] = {
    [COMPARE_EQ] = { "eq", "==" },
    [COMPARE_NEQ] = { "ne", "!=" },

    [COMPARE_LT] = { "lt", "<" },
    [COMPARE_LTE] = { "le", "<=" },
    [COMPARE_GT] = { "gt", ">" },
    [COMPARE_GTE] = { "ge", ">=" },
    
    [COMPARE_AND] = { "and", "&&" },
    [COMPARE_OR] = { "or", "||" }
};

static operand_name_t unary_operands[] = {
    [UNARY_NEG] = { "neg", "-" },
    [UNARY_ABS] = { "abs", "abs" },

    [UNARY_BITFLIP] = { "bitflip", "~" },
    [UNARY_NOT] = { "not", "!" }
};

// operand accessors

operand_name_t binary_operand_name(binary_t op) {
    return binary_operands[op];
}

operand_name_t compare_operand_name(compare_t op) {
    return compare_operands[op];
}

operand_name_t unary_operand_name(unary_t op) {
    return unary_operands[op];
}

// name accessors

const char *binary_name(binary_t op) {
    return binary_operand_name(op).name;
}

const char *compare_name(compare_t op) {
    return compare_operand_name(op).name;
}

const char *unary_name(unary_t op) {
    return unary_operand_name(op).name;
}

// symbol accessors

const char *binary_symbol(binary_t op) {
    return binary_operand_name(op).symbol;
}

const char *compare_symbol(compare_t op) {
    return compare_operand_name(op).symbol;
}

const char *unary_symbol(unary_t op) {
    return unary_operand_name(op).symbol;
}