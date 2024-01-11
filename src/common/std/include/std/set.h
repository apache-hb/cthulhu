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

/// @brief create a new set
///
/// @param size the initial size of the set
/// @param info the type info for the key type
/// @param arena the arena to allocate from
///
/// @return the new set
NODISCARD
set_t *set_new(IN_RANGE(>, 0) size_t size, type_info_t info, IN_NOTNULL arena_t *arena);

/// @brief add a key to a set
/// @pre @p key is not NULL
///
/// @param set the set to add the key to
/// @param key the key to add
///
/// @return the key that was added, or the existing key if it already exists
const void *set_add(IN_NOTNULL set_t *set, IN_NOTNULL const void *key);

/// @brief check if a set contains a key
/// @pre @p key is not NULL
///
/// @param set the set to check
/// @param key the key to check for
///
/// @return true if the set contains the key
NODISCARD CONSTFN
bool set_contains(IN_NOTNULL const set_t *set, IN_NOTNULL const void *key);

/// @brief remove a key from a set
/// @pre @p key is not NULL
///
/// @param set the set to remove the key from
/// @param key the key to remove
void set_delete(IN_NOTNULL set_t *set, IN_NOTNULL const void *key);

/// @brief check if a set is empty
///
/// @param set the set to check
///
/// @return true if the set is empty
NODISCARD CONSTFN
bool set_empty(IN_NOTNULL set_t *set);

/// @brief clear all keys from a set
///
/// @param set the set to clear
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
