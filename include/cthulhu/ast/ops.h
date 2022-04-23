#pragma once

/**
 * @brief all binary operators
 */
typedef enum {
    BINARY_ADD, ///< lhs + rhs
    BINARY_SUB, ///< lhs - rhs
    BINARY_MUL, ///< lhs * rhs
    BINARY_DIV, ///< lhs / rhs
    BINARY_REM, ///< lhs % rhs

    BINARY_AND, ///< lhs && rhs
    BINARY_OR,  ///< lhs || rhs

    BINARY_BITAND, ///< lhs & rhs
    BINARY_BITOR,  ///< lhs | rhs
    BINARY_XOR,    ///< lhs ^ rhs
    BINARY_SHL,    ///< lhs << rhs
    BINARY_SHR,    ///< lhs >> rhs

    BINARY_TOTAL
} binary_t;

/**
 * @brief all comparison operators
 */
typedef enum {
    COMPARE_EQ,  ///< lhs == rhs
    COMPARE_NEQ, ///< lhs != rhs

    COMPARE_LT,  ///< lhs < rhs
    COMPARE_LTE, ///< lhs <= rhs
    COMPARE_GT,  ///< lhs > rhs
    COMPARE_GTE, ///< lhs >= rhs

    COMPARE_TOTAL
} compare_t;

/**
 * @brief all unary operators
 */
typedef enum {
    UNARY_NEG, ///< -operand
    UNARY_ABS, ///< abs(operand)

    UNARY_BITFLIP, ///< ~operand
    UNARY_NOT,     ///< !operand

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
