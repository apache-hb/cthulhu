// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_std_api.h>

#include "core/analyze.h"

#include "std/typeinfo.h"

CT_BEGIN_API

typedef struct arena_t arena_t;
typedef struct typevec_t typevec_t;
typedef struct vector_t vector_t;

/// @defgroup hash_map Unordered map
/// @ingroup standard
/// @brief Hash map
///
/// the map makes some assumptions about keys and values
/// - identical pointers are equal
/// - copying only the pointer is sufficient for copying the value
/// - NULL is the empty value
/// - key lifetimes are externally managed and valid for the lifetime of the map
///
/// the map interface provides a @a map_new and a @a map_optimal function for performance reasons.
/// its recommended to use @a map_optimal if you have an estimate of the number of elements in the map.
///
/// @{

/// @brief a single node in a map
typedef struct bucket_t bucket_t;

/// @brief an unordered hash map
/// @warning this is an opaque type, do not access its members directly aside from 0-initializing it.
typedef struct map_t
{
    /// @brief the arena this map allocates from
    arena_t *arena;

    /// @brief the hash function for this map
    hash_info_t info;

    /// @brief the number of top level buckets
    FIELD_RANGE(>, 0) size_t size;

    /// @brief the number of buckets used
    FIELD_RANGE(<, size) size_t used;

    /// @brief bucket data
    FIELD_SIZE(size) bucket_t *data;
} map_t;

/// @brief an empty map
extern const map_t kEmptyMap;

/// @brief initialize a map
///
/// @param map the map to initialize
/// @param size the initial size of the map
/// @param info the type info for the key type
/// @param arena the arena to allocate from
CT_STD_API void map_init(OUT_NOTNULL map_t *map, IN_DOMAIN(>, 0) size_t size, hash_info_t info, IN_NOTNULL arena_t *arena);

/// @brief create a new map on the stack
///
/// @param size the initial size of the map
/// @param info the type info for the key type
/// @param arena the arena to allocate from
///
/// @return the new map
CT_NODISCARD
CT_STD_API map_t map_make(IN_DOMAIN(>, 0) size_t size, hash_info_t info, IN_NOTNULL arena_t *arena);

/// @brief create a new map on the heap
///
/// @param size the initial size of the map
/// @param info the type info for the key type
/// @param arena the arena to allocate from
///
/// @return the new map
CT_NODISCARD
CT_STD_API map_t *map_new(IN_DOMAIN(>, 0) size_t size, hash_info_t info, IN_NOTNULL arena_t *arena);

/// @brief create a new map with an optimal size
///
/// @param size the estimated number of elements
/// @param info the type info for the key type
/// @param arena the arena to allocate from
///
/// @return the new map
CT_NODISCARD
CT_STD_API map_t *map_optimal(IN_DOMAIN(>, 0) size_t size, hash_info_t info, IN_NOTNULL arena_t *arena);

/// @brief set a key-value pair in a map
/// @pre @p key is not NULL
///
/// @param map the map to set the key-value pair in
/// @param key the key to set
/// @param value the value to set
CT_STD_API void map_set(IN_NOTNULL map_t *map, IN_NOTNULL const void *key, void *value);

/// @brief get a value from a map
///
/// @param map the map to get the value from
/// @param key the key to get the value for
///
/// @return the value for @p key or NULL if the key is not found
CT_NODISCARD CT_PUREFN
CT_STD_API void *map_get(IN_NOTNULL const map_t *map, IN_NOTNULL const void *key);

/// @brief get a value from a map or a default value
///
/// @param map the map to get the value from
/// @param key the key to get the value for
/// @param other the default value to return if the key is not found
///
/// @return the value named by @p key or @p other if the key is not found
CT_NODISCARD CT_PUREFN
CT_STD_API void *map_get_default(IN_NOTNULL const map_t *map, IN_NOTNULL const void *key, void *other);

/// @brief check if a map contains a key
///
/// @param map the map to check
/// @param key the key to check for
///
/// @retval true the map contains @p key
CT_NODISCARD CT_PUREFN
CT_STD_API bool map_contains(IN_NOTNULL const map_t *map, IN_NOTNULL const void *key);

/// @brief delete a key-value pair from a map
/// @note this does no memory management, it only removes the key-value pair from the map
///
/// @param map the map to delete the key-value pair from
/// @param key the key to delete
///
/// @retval true the key-value pair was deleted
CT_STD_API bool map_delete(IN_NOTNULL map_t *map, IN_NOTNULL const void *key);

/// @brief collect all the values from a map into a vector
///
/// @param map the map to collect the values from
///
/// @return a vector containing all the values
CT_NODISCARD
CT_STD_API vector_t *map_values(IN_NOTNULL map_t *map);

/// @brief collect all key-value pairs in a map into a vector
/// returns a typevec_t<map_entry_t>
///
/// @param map the map to collect the key-value pairs from
///
/// @return a vector containing all the key-value pairs
CT_NODISCARD
CT_STD_API typevec_t *map_entries(IN_NOTNULL map_t *map);

/// @brief get the number of key-value pairs in a map
///
/// @param map the map to get the size of
///
/// @return the number of key-value pairs in the map
CT_NODISCARD CT_PUREFN
CT_STD_API size_t map_count(IN_NOTNULL const map_t *map);

/// @brief clear all key-value pairs from a map
/// @note this does no memory management, it only removes all key-value pairs from the map
/// does not free the map itself or shrink its internal storage
///
/// @param map the map to clear
CT_STD_API void map_reset(IN_NOTNULL map_t *map);

/// @brief a key-value pair in a map
typedef struct map_entry_t
{
    const void *key; ///< the key of this entry
    void *value;     ///< the value of this entry
} map_entry_t;

/// @brief a map iterator handle
/// @warning these are internal to the iterator and should not be used directly
typedef struct map_iter_t
{
    const map_t *map;   ///< the map being iterated over
    size_t index; ///< current top level bucket index

    bucket_t *bucket; ///< the current bucket
    bucket_t *next;   ///< the next bucket in the chain
} map_iter_t;

/// @brief create a new map iterator
/// @warning iterators are invalidated by any operation that modifies the map.
/// @note iteration order is unspecified.
///
/// @param map the map to iterate over
///
/// @return the new iterator
CT_NODISCARD CT_PUREFN
CT_STD_API map_iter_t map_iter(IN_NOTNULL const map_t *map);

/// @brief get the next key-value pair from a map iterator
/// @warning this functions behaviour is undefined if called on an iterator that has no more elements
///
/// @param iter the iterator to get the next key-value pair from
///
/// @return the next key-value pair
CT_NODISCARD CT_NOALIAS
CT_STD_API map_entry_t map_next(IN_NOTNULL map_iter_t *iter);

/// @brief check if a map iterator has more elements
///
/// @param iter the iterator to check
///
/// @retval true the iterator has more elements
CT_NODISCARD CT_PUREFN
CT_STD_API bool map_has_next(IN_NOTNULL const map_iter_t *iter);

/// @brief get the next key-value pair from a map iterator
/// returns the values via out parameters for ease of use
/// this also returns false if the iterator has no more elements
/// this is intended to be used via @a CTU_MAP_NEXT for more idiomatic usage
///
/// @param iter the iterator to get the next key-value pair from
/// @param key the key of the next key-value pair
/// @param value the value of the next key-value pair
///
/// @retval true the iterator has more elements
CT_NODISCARD
CT_STD_API bool map_next_pair(IN_NOTNULL map_iter_t *iter, IN_NOTNULL const void **key, IN_NOTNULL void **value);

/// @def CTU_MAP_NEXT(iter, key, value)
/// @brief get the next key-value pair from a map iterator
///
/// intended to be used as such:
/// @code{.c}
/// map_iter_t iter = map_iter(map);
/// const char *key = NULL;
/// void *value = NULL;
/// while (CTU_MAP_NEXT(&iter, &key, &value))
/// {
///     // do something with key and value
/// }
/// @endcode
///
/// @param iter the iterator to get the next key-value pair from
/// @param key the key of the next key-value pair
/// @param value the value of the next key-value pair
///
/// @retval true the iterator has more elements
#define CTU_MAP_NEXT(iter, key, value) map_next_pair(iter, (const void **)(key), (void **)(value))

/// @}

CT_END_API
