#pragma once

#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

/// @brief hash function for a type
///
/// @param ptr the pointer to hash
///
/// @return the hash of the data
typedef size_t (*type_hash_t)(const void *ptr);

/// @brief equals function for a type
///
/// @param lhs the left hand side of the equals
/// @param rhs the right hand side of the equals
///
/// @return if the 2 values are equal
typedef bool (*type_equals_t)(const void *lhs, const void *rhs);

/// @brief type information for a type
typedef struct typeinfo_t
{
    /// @brief the name of the type
    const char *name;

    /// @brief the size of the type
    size_t size;

    /// @brief the hash function for the type
    type_hash_t hash;

    /// @brief the equals function for the type
    type_equals_t equals;
} typeinfo_t;

END_API
