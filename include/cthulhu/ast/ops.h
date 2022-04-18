#pragma once

/**
 * @brief all binary operators
 */
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

/**
 * @brief all comparison operators
 */
typedef enum {
    COMPARE_EQ, // ==
    COMPARE_NEQ, // !=

    COMPARE_LT, // <
    COMPARE_LTE, // <=
    COMPARE_GT, // >
    COMPARE_GTE, // >=

    COMPARE_TOTAL
} compare_t;

/**
 * @brief all unary operators
 */
typedef enum {
    UNARY_NEG,
    UNARY_ABS,

    UNARY_BITFLIP,
    UNARY_NOT,

    UNARY_TOTAL
} unary_t;

/**
 * @brief get the name of a binary operator
 * 
 * @param op the binary operator
 * @return the name
 */
const char *binary_name(binary_t op);

/**
 * @brief get the name of a unary operator
 * 
 * @param op the unary operator
 * @return the name
 */
const char *compare_name(compare_t op);

/**
 * @brief get the name of a unary operator
 * 
 * @param op the unary operator
 * @return the name
 */
const char *unary_name(unary_t op);

/**
 * @brief get the C symbol for a binary operator
 * 
 * @param op the binary operator
 * @return the equivalent C symbol for the operator
 */
const char *binary_symbol(binary_t op);

/**
 * @brief get the C symbol for a unary operator
 * 
 * @param op the unary operator
 * @return the equivalent C symbol for the operator
 */
const char *compare_symbol(compare_t op);

/**
 * @brief get the C symbol for a unary operator
 * 
 * @param op the unary operator
 * @return the equivalent C symbol for the operator
 */
const char *unary_symbol(unary_t op);
