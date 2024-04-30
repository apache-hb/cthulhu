// SPDX-License-Identifier: LGPL-3.0-only
#pragma once

#include <ctu_std_api.h>

#include "core/compiler.h"
#include "core/analyze.h"
#include "core/types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

CT_BEGIN_API

/// @defgroup type_info Type information
/// @ingroup standard
/// @brief Type information
/// @{

/// @brief hash an object pointed to by @p key
///
/// @param key the object to hash
///
/// @return the hash of @p key
typedef ctu_hash_t (*hash_fn_t)(const void *key);

/// @brief compare two objects pointed to by @p lhs and @p rhs
///
/// @param lhs the left hand side of the comparison
/// @param rhs the right hand side of the comparison
///
/// @retval true if @p lhs and @p rhs are equal
/// @retval false otherwise
typedef bool (*equals_fn_t)(const void *lhs, const void *rhs);

/// @brief information for using a type in a hashset or hashmap
typedef struct hash_info_t
{
    /// @brief the size of the type
    FIELD_RANGE(0, SIZE_MAX) size_t size;

    /// @brief the hash function for the type
    hash_fn_t hash;

    /// @brief the equality function for the type
    equals_fn_t equals;
} hash_info_t;

/// @brief type information for a c style string
CT_STD_API extern const hash_info_t kTypeInfoString;

/// @brief type information for a generic pointer
/// this operates on the pointer itself and not the data it points to
CT_STD_API extern const hash_info_t kTypeInfoPtr;

/// @brief type information for a text_view_t
CT_STD_API extern const hash_info_t kTypeInfoText;

/// @}

CT_END_API
