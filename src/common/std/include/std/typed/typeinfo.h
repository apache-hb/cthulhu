#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdbool.h>

BEGIN_API

typedef enum strong_ordering_t
{
    eStrongOrderingLess,
    eStrongOrderingEqual,
    eStrongOrderingGreater,

    eStrongOrderingCount
} strong_ordering_t;

/// @brief hash function for a type
///
/// @param ptr the pointer to hash
///
/// @return the hash of the data
typedef size_t (*type_hash_t)(const void *ptr);

/// @brief compare function for a type
///
/// @param lhs the left hand side of the compare
/// @param rhs the right hand side of the compare
///
/// @return if the 2 values are equal
typedef strong_ordering_t (*type_compare_strong_t)(const void *lhs, const void *rhs);

/// @brief copy function for a type
///
/// @param dst the destination of the copy
/// @param src the source of the copy
typedef void (*type_copy_t)(void *dst, const void *src);

/// @brief type information for a type
typedef struct typeinfo_t
{
    /// @brief the name of the type
    const char *name;

    /// @brief the size of the type
    FIELD_RANGE(>, 0) size_t size;

    /// @brief a pointer to the empty value for the type
    const void *empty;

    /// @brief the hash function for the type
    type_hash_t fn_hash;

    /// @brief the compare function for the type
    type_compare_strong_t fn_compare;
} typeinfo_t;

extern const typeinfo_t kPointerTypeInfo;
extern const typeinfo_t kStringTypeInfo;

END_API
