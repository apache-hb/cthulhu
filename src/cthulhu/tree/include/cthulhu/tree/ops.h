#pragma once

#include <ctu_tree_api.h>

#include "core/compiler.h"
#include "core/analyze.h"

BEGIN_API

/// @ingroup tree
/// @{

/// @brief all type qualifiers
typedef enum quals_t
{
#define TYPE_QUALIFIER(ID, NAME, FLAG) ID = (FLAG),
#include "tree.inc"
} quals_t;

/// @brief all binary operators
typedef enum binary_t
{
#define BINARY_OP(ID, NAME, SYMBOL) ID,
#include "tree.inc"
    eBinaryTotal
} binary_t;

/// @brief all comparison operators
typedef enum compare_t
{
#define COMPARE_OP(ID, NAME, SYMBOL) ID,
#include "tree.inc"
    eCompareTotal
} compare_t;

/// @brief all unary operators
typedef enum unary_t
{
#define UNARY_OP(ID, NAME, SYMBOL) ID,
#include "tree.inc"
    eUnaryTotal
} unary_t;

/// @brief all casts
typedef enum cast_t
{
#define CAST_OP(ID, NAME) ID,
#include "tree.inc"
    eCastTotal
} cast_t;

/// @brief all builtins
typedef enum builtin_t
{
#define TREE_BUILTIN(ID, NAME) ID,
#include "tree.inc"
    eBuiltinTotal
} builtin_t;

/// @brief all arities
typedef enum arity_t
{
#define TREE_ARITY(ID, STR) ID,
#include "tree.inc"
    eArityTotal
} arity_t;

/// @brief the linkage of a declaration
typedef enum tree_link_t
{
#define TREE_LINKAGE(ID, STR) ID,
#include "tree.inc"
    eLinkTotal
} tree_link_t;

/// @brief the type of jump
typedef enum tree_jump_t
{
#define TREE_JUMP(ID, STR) ID,
#include "tree.inc"
    eJumpTotal
} tree_jump_t;

/// @brief symbol visibility
typedef enum visibility_t
{
#define TREE_VISIBILITY(ID, STR) ID,
#include "tree.inc"
    eVisibileTotal
} visibility_t;

/// @brief digit width
typedef enum digit_t
{
#define DIGIT_KIND(ID, STR) ID,
#include "tree.inc"
    eDigitTotal
} digit_t;

/// @brief integer sign
typedef enum sign_t
{
#define SIGN_KIND(ID, STR) ID,
#include "tree.inc"
    eSignTotal
} sign_t;

/// @brief get the pretty name of a unary operator
///
/// @param op the operator to get the name of
///
/// @return the name of @p op
RET_NOTNULL
CT_TREE_API const char *unary_name(IN_RANGE(<, eUnaryTotal) unary_t op);

/// @brief get the pretty name of a binary operator
///
/// @param op the operator to get the name of
///
/// @return the name of @p op
RET_NOTNULL
CT_TREE_API const char *binary_name(IN_RANGE(<, eBinaryTotal) binary_t op);

/// @brief get the pretty name of a comparison operator
///
/// @param op the operator to get the name of
///
/// @return the name of @p op
RET_NOTNULL
CT_TREE_API const char *compare_name(IN_RANGE(<, eCompareTotal)compare_t op);

/// @brief get the C symbol of a unary operator
///
/// @param op the operator to get the symbol of
///
/// @return the symbol of @p op
RET_NOTNULL
CT_TREE_API const char *unary_symbol(IN_RANGE(<, eUnaryTotal) unary_t op);

/// @brief get the C symbol of a binary operator
///
/// @param op the operator to get the symbol of
///
/// @return the symbol of @p op
RET_NOTNULL
CT_TREE_API const char *binary_symbol(IN_RANGE(<, eBinaryTotal)binary_t op);

/// @brief get the C symbol of a comparison operator
///
/// @param op the operator to get the symbol of
///
/// @return the symbol of @p op
RET_NOTNULL
CT_TREE_API const char *compare_symbol(IN_RANGE(<, eCompareTotal) compare_t op);

/// @brief get the pretty name of a integer sign
///
/// @param sign the sign to get the name of
///
/// @return the name of @p op
RET_NOTNULL
CT_TREE_API const char *sign_name(IN_RANGE(<, eSignTotal) sign_t sign);

/// @brief get the pretty name of a digit
///
/// @param digit the digit to get the name of
///
/// @return the name of @p op
RET_NOTNULL
CT_TREE_API const char *digit_name(IN_RANGE(<, eDigitTotal) digit_t digit);

/// @brief get the name of a set of qualifiers
///
/// @param quals the qualifiers to get the name of
///
/// @return the name of @p quals
RET_NOTNULL
CT_TREE_API const char *quals_name(quals_t quals);

/// @brief get the name of a linkage
///
/// @param link the linkage to get the name of
///
/// @return the name of @p link
RET_NOTNULL
CT_TREE_API const char *link_name(IN_RANGE(<, eLinkTotal) tree_link_t link);

/// @brief get the name of visibility
///
/// @param vis the visibility to get the name of
///
/// @return the name of @p vis
RET_NOTNULL
CT_TREE_API const char *vis_name(IN_RANGE(<, eVisibileTotal) visibility_t vis);

/// @}

END_API
