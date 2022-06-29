#pragma once

/**
 * @brief all binary operators
 */
typedef enum
{
    eBinaryAdd, ///< lhs + rhs
    eBinarySub, ///< lhs - rhs
    eBinaryMul, ///< lhs * rhs
    eBinaryDiv, ///< lhs / rhs
    eBinaryRem, ///< lhs % rhs

    eBinaryAnd, ///< lhs && rhs
    eBinaryOr,  ///< lhs || rhs

    eBinaryBitAnd, ///< lhs & rhs
    eBinaryBitOr,  ///< lhs | rhs
    eBinaryXor,    ///< lhs ^ rhs
    eBinaryShl,    ///< lhs << rhs
    eBinaryShr,    ///< lhs >> rhs

    eBinaryTotal
} binary_t;

/**
 * @brief all comparison operators
 */
typedef enum
{
    eCompareEq,  ///< lhs == rhs
    eCompareNeq, ///< lhs != rhs

    eCompareLt,  ///< lhs < rhs
    eCompareLte, ///< lhs <= rhs
    eCompareGt,  ///< lhs > rhs
    eCompareGte, ///< lhs >= rhs

    eCompareTotal
} compare_t;

/**
 * @brief all unary operators
 */
typedef enum
{
    eUnaryNeg, ///< -operand
    eUnaryAbs, ///< abs(operand)

    eUnaryBitflip, ///< ~operand
    eUnaryNot,     ///< !operand

    eUnaryTotal
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
