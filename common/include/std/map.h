#pragma once

#include "base/analyze.h"

#include <stdbool.h>
#include <stddef.h>

BEGIN_API

typedef struct vector_t vector_t;

typedef struct bucket_t bucket_t;
typedef struct map_t map_t;

/**
 * create a new map
 *
 * @param size the number of toplevel buckets used.
 *             increasing this value reduces hash collisions
 *             but increases memory usage and initialization time.
 *
 * @return a new map
 */
NODISCARD
map_t *map_new(size_t size);

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
void map_set(map_t *map, const char *key, void *value);

/**
 * get a value from a map
 *
 * @param map the map to get the value from
 * @param key the key to get the value for
 *
 * @return the value for the key or NULL if the key is not found
 */
NODISCARD CONSTFN void *map_get(map_t *map, const char *key);

/**
 * @brief get a value from a map or a default value if the key is not found
 *
 * @param map the map to get the value from
 * @param key the key to get the value for
 * @param other the default value to return if the key is not found
 *
 * @return the value for the key or the default value if the key is not found
 */
NODISCARD CONSTFN void *map_get_default(map_t *map, const char *key, void *other);

void map_set_ptr(map_t *map, const void *key, void *value);

NODISCARD CONSTFN void *map_get_ptr(map_t *map, const void *key);

NODISCARD CONSTFN void *map_get_default_ptr(map_t *map, const void *key, void *other);

/**
 * @brief collect all the values in a map into a vector
 *
 * @param map the map to collect the values from
 *
 * @return a vector containing all the values
 */
NODISCARD
vector_t *map_values(map_t *map);

NODISCARD
vector_t *map_entries(map_t *map);

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

NODISCARD CONSTFN map_iter_t map_iter(map_t *map);

NODISCARD CONSTFN map_entry_t map_next(map_iter_t *iter);

NODISCARD CONSTFN bool map_has_next(map_iter_t *iter);

void map_reset(map_t *map);

END_API
