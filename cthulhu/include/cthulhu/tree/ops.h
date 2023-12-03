#pragma once

typedef enum quals_t {
    eQualUnknown = (0 << 0), ///< defer to the inner type for the qualifiers

    eQualConst = (1 << 0),
    eQualMutable = (1 << 1),
    eQualVolatile = (1 << 2),
    eQualAtomic = (1 << 3),
} quals_t;

/**
 * @brief all binary operators
 */
typedef enum binary_t {
#define BINARY_OP(ID, NAME, SYMBOL) ID,
#include "tree.inc"
    eBinaryTotal
} binary_t;

/**
 * @brief all comparison operators
 */
typedef enum compare_t {
#define COMPARE_OP(ID, NAME, SYMBOL) ID,
#include "tree.inc"
    eCompareTotal
} compare_t;

/**
 * @brief all unary operators
 */
typedef enum unary_t {
#define UNARY_OP(ID, NAME, SYMBOL) ID,
#include "tree.inc"
    eUnaryTotal
} unary_t;

typedef enum cast_t {
#define CAST_OP(ID, NAME) ID,
#include "tree.inc"
    eCastTotal
} cast_t;

typedef enum builtin_t {
#define TREE_BUILTIN(ID, NAME) ID,
#include "tree.inc"
    eBuiltinTotal
} builtin_t;

typedef enum arity_t {
#define TREE_ARITY(ID, STR) ID,
#include "tree.inc"
    eArityTotal
} arity_t;

/**
 * @brief the visibility of a declaration
 */
typedef enum tree_link_t {
#define TREE_LINKAGE(ID, STR) ID,
#include "tree.inc"
    eLinkTotal
} tree_link_t;

typedef enum tree_jump_t {
#define TREE_JUMP(ID, STR) ID,
#include "tree.inc"
    eJumpTotal
} tree_jump_t;

typedef enum visibility_t {
#define TREE_VISIBILITY(ID, STR) ID,
#include "tree.inc"
    eVisibileTotal
} visibility_t;

typedef enum digit_t {
#define DIGIT_KIND(ID, STR) ID,
#include "tree.inc"
    eDigitTotal
} digit_t;

typedef enum sign_t {
#define SIGN_KIND(ID, STR) ID,
#include "tree.inc"
    eSignTotal
} sign_t;

const char *unary_name(unary_t op);
const char *binary_name(binary_t op);
const char *compare_name(compare_t op);

const char *unary_symbol(unary_t op);
const char *binary_symbol(binary_t op);
const char *compare_symbol(compare_t op);

const char *sign_name(sign_t sign);
const char *digit_name(digit_t digit);

const char *quals_name(quals_t quals);
const char *link_name(tree_link_t link);
const char *vis_name(visibility_t vis);
