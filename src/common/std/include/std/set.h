#pragma once

#include "core/analyze.h"

#include "std/typed/info.h"

BEGIN_API

/// @defgroup Set Unordered set
/// @ingroup Standard
/// @brief Hash set
/// @{

typedef struct set_t set_t;
typedef struct item_t item_t;
typedef struct arena_t arena_t;

set_t *set_new_info(size_t size, type_info_t info, arena_t *arena);

const void *set_add_ex(IN_NOTNULL set_t *set, const void *key);
bool set_contains_ex(IN_NOTNULL const set_t *set, const void *key);
void set_delete_ex(IN_NOTNULL set_t *set, const void *key);

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

/// @}

END_API
