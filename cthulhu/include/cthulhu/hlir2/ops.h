#pragma once

/**
 * @brief all binary operators
 */
typedef enum binary_t {
#define BINARY_OP(ID, NAME, SYMBOL) ID,
#include "hlir.inc"
    eBinaryTotal
} binary_t;

/**
 * @brief all comparison operators
 */
typedef enum compare_t {
#define COMPARE_OP(ID, NAME, SYMBOL) ID,
#include "hlir.inc"
    eCompareTotal
} compare_t;

/**
 * @brief all unary operators
 */
typedef enum unary_t {
#define UNARY_OP(ID, NAME, SYMBOL) ID,
#include "hlir.inc"
    eUnaryTotal
} unary_t;

typedef enum cast_t {
#define CAST_OP(ID, NAME) ID,
#include "hlir.inc" 
    eCastTotal
} cast_t;

typedef enum builtin_t {
#define HLIR_BUILTIN(ID, NAME) ID,
#include "hlir.inc"
    eBuiltinTotal
} builtin_t;
