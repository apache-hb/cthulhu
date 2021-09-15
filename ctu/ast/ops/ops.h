#pragma once

//
// unary and binary operands
//

typedef enum {
    BINARY_ADD,
    BINARY_SUB,
    BINARY_MUL,
    BINARY_DIV,
    BINARY_REM,

    BINARY_EQ,
    BINARY_NEQ,
    BINARY_GT,
    BINARY_GTE,
    BINARY_LT,
    BINARY_LTE,

    BINARY_AND,
    BINARY_OR,
    BINARY_XOR,
    BINARY_SHL,
    BINARY_SHR,
    BINARY_BITAND,
    BINARY_BITOR,
} binary_t;

typedef enum {
    UNARY_NEG,
    UNARY_ABS,
} unary_t;

//
// get the pretty name of a unary or binary operand
//

const char *binary_name(binary_t op);
const char *unary_name(unary_t op);
