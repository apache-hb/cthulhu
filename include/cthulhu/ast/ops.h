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

    BINARY_AND, // logical and
    BINARY_OR, // logical or

    BINARY_BITAND, // bitwise and
    BINARY_BITOR, // bitwise or
    BINARY_XOR, // bitwise xor
    BINARY_SHL, // bitwise shift left
    BINARY_SHR, // bitwise shift right

    BINARY_TOTAL
} binary_t;

typedef enum {
    COMPARE_EQ, // ==
    COMPARE_NEQ, // !=

    COMPARE_LT, // <
    COMPARE_LTE, // <=
    COMPARE_GT, // >
    COMPARE_GTE, // >=

    COMPARE_TOTAL
} compare_t;

typedef enum {
    UNARY_NEG,
    UNARY_ABS,

    UNARY_BITFLIP,
    UNARY_NOT,

    UNARY_TOTAL
} unary_t;

//
// get the pretty name of a unary or binary operand
//

typedef struct {
    const char *name;
    const char *symbol;
} operand_name_t;

operand_name_t binary_operand_name(binary_t op);
operand_name_t compare_operand_name(compare_t op);
operand_name_t unary_operand_name(unary_t op);

const char *binary_name(binary_t op);
const char *compare_name(compare_t op);
const char *unary_name(unary_t op);

const char *binary_symbol(binary_t op);
const char *compare_symbol(compare_t op);
const char *unary_symbol(unary_t op);
