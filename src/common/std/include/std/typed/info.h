#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdbool.h>
#include <stddef.h>

BEGIN_API

/// @defgroup type_info Type information
/// @ingroup standard
/// @brief Type information
/// @{

/// @brief hash an object pointed to by @p key
///
/// @param key the object to hash
///
/// @return the hash of @p key
typedef size_t (*hash_fn_t)(const void *key);

/// @brief compare two objects pointed to by @p lhs and @p rhs
///
/// @param lhs the left hand side of the comparison
/// @param rhs the right hand side of the comparison
///
/// @retval true if @p lhs and @p rhs are equal
/// @retval false otherwise
typedef bool (*equals_fn_t)(const void *lhs, const void *rhs);

/// @brief type information for a type
typedef struct typeinfo_t
{
    /// @brief the size of the type
    FIELD_RANGE(>, 0) size_t size;

    /// @brief the hash function for the type
    hash_fn_t hash;

    /// @brief the equality function for the type
    equals_fn_t equals;
} typeinfo_t;

/// @brief type information for a c style string
extern const typeinfo_t kTypeInfoString;

/// @brief type information for a generic pointer
/// this operates on the pointer itself and not the data it points to
extern const typeinfo_t kTypeInfoPtr;

/// @}

END_API
