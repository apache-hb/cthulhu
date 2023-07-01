#pragma once


typedef enum quals_t {
    eQualDefault = (0 << 0),
    eQualMutable = (1 << 0),
    eQualVolatile = (1 << 1),
    eQualAtomic = (1 << 2),
} quals_t;

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

typedef enum arity_t {
#define HLIR_ARITY(ID, STR) ID,
#include "hlir.inc"
    eArityTotal
} arity_t;
/**
 * @brief the visibility of a declaration
 */
typedef enum h2_link_t {
#define HLIR_LINKAGE(ID, STR) ID,
#include "hlir.inc"
    eLinkTotal
} h2_link_t;

typedef enum h2_visible_t {
#define HLIR_VISIBILITY(ID, STR) ID,
#include "hlir.inc"
    eHlirVisibilityTotal
} h2_visible_t;

typedef enum digit_t {
#define DIGIT_KIND(ID, STR) ID,
#include "hlir.inc"
    eDigitTotal
} digit_t;

typedef enum sign_t {
#define SIGN_KIND(ID, STR) ID,
#include "hlir.inc"
    eSignTotal
} sign_t;

const char *unary_name(unary_t op);
const char *binary_name(binary_t op);
const char *compare_name(compare_t op);

const char *sign_name(sign_t sign);
const char *digit_name(digit_t digit);

const char *quals_name(quals_t quals);
