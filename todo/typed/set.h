#pragma once

#include "base/macros.h"

#include <stdbool.h>

BEGIN_API

typedef struct typeset_t typeset_t;
typedef struct typeset_item_t typeset_item_t;

/**
 * @brief a hash function for a type inside the typeset
 *
 * @param value the value to hash, will never be NULL
 */
typedef size_t (*typeset_hash_t)(const void *value);

/**
 * @brief a equality function for a type inside the typeset
 *
 * @param lhs the left hand side of the equality, never NULL
 * @param rhs the right hand side of the equality, never NULL
 * @return true if the two values are equal
 */
typedef bool (*typeset_equal_t)(const void *lhs, const void *rhs);

typedef struct typeset_info_t {
    size_t typeSize; ///< the size of the type, must be > 0

    typeset_hash_t fnHash; ///< the hash function for the type, must not be NULL
    typeset_equal_t fnEqual; ///< the equality function for the type, must not be NULL
} typeset_info_t;


///
/// typeset api
///

NODISCARD
typeset_t *typeset_new(size_t size, IN_NOTNULL const typeset_info_t *info);

const void *typeset_add(IN_NOTNULL typeset_t *set, const void *key);

NODISCARD CONSTFN
bool typeset_contains(IN_NOTNULL typeset_t *set, const void *key);

NODISCARD CONSTFN
bool typeset_empty(IN_NOTNULL typeset_t *set);

void typeset_reset(IN_NOTNULL typeset_t *set);

///
/// typeset iter api
///

typedef struct typeset_iter_t {
    typeset_t *set;
    size_t index;

    typeset_item_t *current;
    typeset_item_t *next;
} typeset_iter_t;

NODISCARD CONSTFN
typeset_iter_t typeset_iter(IN_NOTNULL typeset_t *set);

NODISCARD
const void *typeset_next(IN_NOTNULL typeset_iter_t *iter);

NODISCARD CONSTFN
bool typeset_has_next(IN_NOTNULL typeset_iter_t *iter);

END_API
