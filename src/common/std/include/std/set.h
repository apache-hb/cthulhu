#pragma once

#include "core/analyze.h"

#include "std/typed/info.h"

BEGIN_API

typedef struct arena_t arena_t;

/// @defgroup hash_set Unordered set
/// @ingroup standard
/// @brief Hash set
/// @{

/// @brief an unordered hash set
typedef struct set_t set_t;

/// @brief a single node in a set
typedef struct item_t item_t;

/// @brief create a new set
///
/// @param size the initial size of the set
/// @param info the type info for the key type
/// @param arena the arena to allocate from
///
/// @return the new set
NODISCARD
CT_STD_API set_t *set_new(IN_RANGE(>, 0) size_t size, typeinfo_t info, IN_NOTNULL arena_t *arena);

/// @brief add a key to a set
/// @pre @p key is not NULL
///
/// @param set the set to add the key to
/// @param key the key to add
///
/// @return the key that was added, or the existing key if it already exists
CT_STD_API const void *set_add(IN_NOTNULL set_t *set, IN_NOTNULL const void *key);

/// @brief check if a set contains a key
/// @pre @p key is not NULL
///
/// @param set the set to check
/// @param key the key to check for
///
/// @return true if the set contains the key
NODISCARD CONSTFN
CT_STD_API bool set_contains(IN_NOTNULL const set_t *set, IN_NOTNULL const void *key);

/// @brief remove a key from a set
/// @pre @p key is not NULL
///
/// @param set the set to remove the key from
/// @param key the key to remove
CT_STD_API void set_delete(IN_NOTNULL set_t *set, IN_NOTNULL const void *key);

/// @brief check if a set is empty
///
/// @param set the set to check
///
/// @return true if the set is empty
NODISCARD CONSTFN
CT_STD_API bool set_empty(IN_NOTNULL set_t *set);

/// @brief clear all keys from a set
///
/// @param set the set to clear
CT_STD_API void set_reset(IN_NOTNULL set_t *set);

/// @brief a set iterator handle
/// @warning these are internal fields and should not be used directly
typedef struct set_iter_t
{
    set_t *set; ///< the set to iterate over
    size_t index; ///< the current bucket index

    item_t *current; ///< the current item
    item_t *next; ///< the next item
} set_iter_t;

/// @brief acquire a set iterator for a set
/// @warning modifying the set invalidates all iterators
/// @note the iteration order is unspecified.
///
/// @param set the set to iterate over
///
/// @return the new iterator
NODISCARD CONSTFN
CT_STD_API set_iter_t set_iter(IN_NOTNULL set_t *set);

/// @brief get the next item from a set iterator
/// @warning this functions behaviour is undefined if called on an iterator that has no more items
///
/// @param iter the iterator to get the next item from
///
/// @return the next item
NODISCARD
CT_STD_API const void *set_next(IN_NOTNULL set_iter_t *iter);

/// @brief check if a set iterator has more items
///
/// @param iter the iterator to check
///
/// @retval true the iterator has more items
/// @retval false the iterator has no more items
NODISCARD CONSTFN
CT_STD_API bool set_has_next(IN_NOTNULL set_iter_t *iter);

/// @}

END_API
