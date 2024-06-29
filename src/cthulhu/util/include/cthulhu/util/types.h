// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_util_api.h>

#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct tree_t tree_t;
typedef struct tree_cookie_t tree_cookie_t;

CT_BEGIN_API

/// @ingroup runtime_util
/// @{

/// @brief compare two types for strict equality
/// compares two types for exact equality, does not follow typedefs
///
/// @param lhs the left hand side type
/// @param rhs the right hand side type
///
/// @return true if the types are equal, false otherwise
CT_UTIL_API bool util_types_equal(const tree_t *lhs, const tree_t *rhs);

/// @brief query two types for comparability in binary logic operations
///
/// @param lhs the left hand side type
/// @param rhs the right hand side type
///
/// @return true if the types are comparable, false otherwise
CT_UTIL_API bool util_types_comparable(tree_cookie_t *cookie, const tree_t *lhs, const tree_t *rhs);

/// @brief attempt to cast an expression to a type
///
/// @param dst the desired type
/// @param expr the expression to try and cast
///
/// @return tree_t* the casted expression or @a tree_error if the cast could not be done
CT_UTIL_API tree_t *util_type_cast(const tree_t *dst, tree_t *expr);

/// @brief check if the length of an array is bounded
///
/// @param length the length of the array
///
/// @return true if the length is bounded, false otherwise
CT_UTIL_API bool util_length_bounded(size_t length);

/// @brief get the pretty name of a length
/// return either the length as a string or "unbounded" if the length is unbounded
///
/// @param length the length to get the name of
///
/// @return the name of the length
CT_UTIL_API const char *util_length_name(size_t length);

CT_UTIL_API bool util_type_is_aggregate(const tree_t *type);

/// @}

CT_END_API
