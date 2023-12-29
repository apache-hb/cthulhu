#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>

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

map_t *map_new_arena(size_t size, IN_NOTNULL arena_t *arena);

map_t *map_optimal_arena(size_t size, IN_NOTNULL arena_t *arena);

/**
 * create a map with an optimal number of buckets
 * for a given size
 *
 * @param size the estimated number of elements to store
 *
 * @return a new map
 */
NODISCARD
map_t *map_optimal(size_t size);

/**
 * set or overwrite a value in a map
 *
 * @param map the map to set the value in
 * @param key the key to set the value for
 * @param value the value to set
 */
void map_set(IN_NOTNULL map_t *map, IN_STRING const char *key, void *value);
void map_set_ptr(IN_NOTNULL map_t *map, const void *key, void *value);

/**
 * get a value from a map
 *
 * @param map the map to get the value from
 * @param key the key to get the value for
 *
 * @return the value for the key or NULL if the key is not found
 */
NODISCARD CONSTFN
void *map_get(IN_NOTNULL map_t *map, IN_STRING const char *key);


NODISCARD CONSTFN
void *map_get_ptr(IN_NOTNULL map_t *map, const void *key);

/**
 * @brief get a value from a map or a default value if the key is not found
 *
 * @param map the map to get the value from
 * @param key the key to get the value for
 * @param other the default value to return if the key is not found
 *
 * @return the value for the key or the default value if the key is not found
 */
NODISCARD CONSTFN
void *map_get_default(IN_NOTNULL map_t *map, IN_STRING const char *key, void *other);

NODISCARD CONSTFN
void *map_get_default_ptr(IN_NOTNULL map_t *map, const void *key, void *other);

NODISCARD CONSTFN
bool map_contains(IN_NOTNULL map_t *map, IN_STRING const char *key);

NODISCARD CONSTFN
bool map_contains_ptr(IN_NOTNULL map_t *map, const void *key);

void map_delete(IN_NOTNULL map_t *map, IN_STRING const char *key);

void map_delete_ptr(IN_NOTNULL map_t *map, const void *key);

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

NODISCARD CONSTFN
bool map_empty(IN_NOTNULL map_t *map);

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

NODISCARD CONSTFN
bool map_has_next(IN_NOTNULL map_iter_t *iter);

void map_reset(IN_NOTNULL map_t *map);

/// @}

END_API