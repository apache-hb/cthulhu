#pragma once

/**
 * @brief all binary operators
 */
typedef enum
{
#define BINARY_OP(ID, NAME, SYMBOL) ID,
#include "hlir-def.inc"
    eBinaryTotal
} binary_t;

/**
 * @brief all comparison operators
 */
typedef enum
{
#define COMPARE_OP(ID, NAME, SYMBOL) ID,
#include "hlir-def.inc"
    eCompareTotal
} compare_t;

/**
 * @brief all unary operators
 */
typedef enum
{
#define UNARY_OP(ID, NAME, SYMBOL) ID,
#include "hlir-def.inc"
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
