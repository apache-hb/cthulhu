#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

BEGIN_API
/**
 * @defgroup Set
 * @brief Simple unordered set
 * @{
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
set_t *set_new(size_t size);

/**
 * @brief add a string to a set
 *
 * @param set the set to add to
 * @param key the key to add
 * @return a pointer to the deduplicated key
 */
const char *set_add(IN_NOTNULL set_t *set, IN_NOTNULL const char *key);

/**
 * @brief add a pointer to a set
 *
 * @param set the set to add to
 * @param key the key to add
 * @return a pointer to the deduplicated key
 */
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

/**
 * @brief check if a set contains a ptr key
 *
 * @param set the set to check
 * @param key the key to check
 * @return true if the set contains the key
 * @return false if the set does not contain the key
 */
NODISCARD CONSTFN
bool set_contains_ptr(IN_NOTNULL set_t *set, const void *key);

/**
 * @brief check if a set is empty
 *
 * @param set the set to check
 * @return true if the set is empty
 * @return false if the set is not empty
 */
NODISCARD CONSTFN
bool set_empty(IN_NOTNULL set_t *set);

/**
 * @brief reset a set to empty
 *
 * @param set the set to reset
 */
void set_reset(IN_NOTNULL set_t *set);

typedef struct set_iter_t
{
    set_t *set; ///< the set to iterate over
    size_t index; ///< the current bucket index

    item_t *current; ///< the current item
    item_t *next; ///< the next item
} set_iter_t;

/**
 * @brief begin iterating over a set
 *
 * @param set the set to iterate over
 * @return a set iterator
 */
NODISCARD CONSTFN
set_iter_t set_iter(IN_NOTNULL set_t *set);

/**
 * @brief get the next item in a set iterator
 *
 * @note always call @ref set_has_next before calling this function
 * @param iter the iterator to get the next item from
 * @return the next item
 */
NODISCARD
const void *set_next(IN_NOTNULL set_iter_t *iter);

/**
 * @brief check if a set iterator has a next item
 *
 * @param iter the iterator to check
 * @return true if the iterator has a next item
 * @return false if the iterator does not have a next item
 */
NODISCARD CONSTFN
bool set_has_next(IN_NOTNULL set_iter_t *iter);

/** @} */

END_API
