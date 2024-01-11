#pragma once

#include "core/analyze.h"

#include "std/typed/info.h"

BEGIN_API

/// @defgroup Map Unordered map
/// @ingroup Standard
/// @brief Hash map
/// @{

typedef struct vector_t vector_t;
typedef struct typevec_t typevec_t;
typedef struct bucket_t bucket_t;
typedef struct map_t map_t;
typedef struct arena_t arena_t;

///
/// the map makes some assumptions about keys
/// - identical pointers are equal
/// - copying the pointer is allowed
/// - NULL is the empty value
/// - key lifetimes are externally managed and valid for the lifetime of the map
///

/**
 * create a map with an optimal number of buckets
 * for a given size
 *
 * @param size the estimated number of elements to store
 *
 * @return a new map
 */

map_t *map_new_info(size_t size, type_info_t info, IN_NOTNULL arena_t *arena);
map_t *map_optimal_info(size_t size, type_info_t info, IN_NOTNULL arena_t *arena);

void map_set_ex(IN_NOTNULL map_t *map, IN_NOTNULL const void *key, void *value);
void *map_get_ex(IN_NOTNULL const map_t *map, IN_NOTNULL const void *key);
void *map_get_default_ex(IN_NOTNULL const map_t *map, IN_NOTNULL const void *key, void *other);
bool map_contains_ex(IN_NOTNULL const map_t *map, IN_NOTNULL const void *key);
void map_delete_ex(IN_NOTNULL map_t *map, IN_NOTNULL const void *key);

/**
 * @brief collect all the values in a map into a vector
 *
 * @param map the map to collect the values from
 *
 * @return a vector containing all the values
 */
NODISCARD
vector_t *map_values(IN_NOTNULL map_t *map);

NODISCARD
typevec_t *map_entries(IN_NOTNULL map_t *map);

/// @brief get the number of key-value pairs in a map
///
/// @param map the map to get the size of
///
/// @return the number of key-value pairs in the map
NODISCARD
size_t map_count(IN_NOTNULL map_t *map);

typedef struct map_entry_t
{
    const void *key; ///< the key of this entry
    void *value;     ///< the value of this entry
} map_entry_t;

typedef struct map_iter_t
{
    map_t *map;   ///< the map being iterated over
    size_t index; ///< current top level bucket index

    bucket_t *bucket; ///< the current bucket
    bucket_t *next;   ///< the next bucket in the chain
} map_iter_t;

NODISCARD CONSTFN
map_iter_t map_iter(IN_NOTNULL map_t *map);

NODISCARD
map_entry_t map_next(IN_NOTNULL map_iter_t *iter);

NODISCARD
bool map_next_ex(IN_NOTNULL map_iter_t *iter, const void **key, void **value);

#define CTU_MAP_NEXT(iter, key, value) map_next_ex(iter, (const void **)(key), (void **)(value))

NODISCARD CONSTFN
bool map_has_next(IN_NOTNULL map_iter_t *iter);

void map_reset(IN_NOTNULL map_t *map);

/// @}

END_API
