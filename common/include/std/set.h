#pragma once

#include "base/analyze.h"

#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

BEGIN_API

/**
 * @brief a hashset
 */
typedef struct set_t set_t;

typedef struct item_t item_t;

/**
 * @brief create a new set with a given number of buckets
 *
 * @param size the number of buckets
 * @return the created set
 */
NODISCARD
set_t *set_new(IN_RANGE(0, SIZE_MAX) size_t size);

/**
 * @brief add a string to a set
 *
 * @param set the set to add to
 * @param key the key to add
 * @return a pointer to the deduplicated key
 */
const char *set_add(IN_NOTNULL set_t *set, IN_NOTNULL const char *key);

const void *set_add_ptr(IN_NOTNULL set_t *set, const void *key);

/**
 * @brief check if a set contains a key
 *
 * @param set the set to check
 * @param key the key to check
 * @return true if the set contains the key
 */
NODISCARD CONSTFN 
bool set_contains(IN_NOTNULL set_t *set, IN_STRING const char *key);

NODISCARD CONSTFN 
bool set_contains_ptr(IN_NOTNULL set_t *set, const void *key);

NODISCARD CONSTFN
bool set_empty(IN_NOTNULL set_t *set);

void set_reset(IN_NOTNULL set_t *set);

typedef struct set_iter_t
{
    set_t *set;
    size_t index;

    item_t *current;
    item_t *next;
} set_iter_t;

NODISCARD CONSTFN
set_iter_t set_iter(IN_NOTNULL set_t *set);

NODISCARD 
const void *set_next(IN_NOTNULL set_iter_t *iter);

NODISCARD CONSTFN
bool set_has_next(IN_NOTNULL set_iter_t *iter);

END_API
