#pragma once

#include "cthulhu/util/vector.h"

#include <stddef.h>
#include <stdbool.h>

/**
 * a bucket in a hashmap
 */
typedef struct bucket_t
{
    const void *key;       ///< the key
    void *value;           ///< any pointer value
    struct bucket_t *next; ///< the next bucket in the chain
} bucket_t;

/**
 * a hashmap
 *
 * freeing the map will not free the keys or the values.
 * these need to be freed beforehand by the owner of the container.
 */
typedef struct
{
    size_t size;                      ///< the number of buckets in the toplevel
    FIELD_SIZE(size) bucket_t data[]; ///< the buckets
} map_t;

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
NODISCARD
void *map_get(map_t *map, const char *key);

/**
 * @brief get a value from a map or a default value if the key is not found
 *
 * @param map the map to get the value from
 * @param key the key to get the value for
 * @param other the default value to return if the key is not found
 *
 * @return the value for the key or the default value if the key is not found
 */
void *map_get_default(map_t *map, const char *key, void *other);

void map_set_ptr(map_t *map, const void *key, void *value);
void *map_get_ptr(map_t *map, const void *key);
void *map_get_default_ptr(map_t *map, const void *key, void *other);

/**
 * @brief collect all the values in a map into a vector
 *
 * @param map the map to collect the values from
 *
 * @return a vector containing all the values
 */
NODISCARD
vector_t *map_values(map_t *map);

typedef struct
{
    const void *key; ///< the key of this entry
    void *value;     ///< the value of this entry
} map_entry_t;

typedef struct
{
    map_t *map;   ///< the map being iterated over
    size_t index; ///< current top level bucket index

    bucket_t *bucket; ///< the current bucket
    bucket_t *next;   ///< the next bucket in the chain
} map_iter_t;

NODISCARD
map_iter_t map_iter(map_t *map);

NODISCARD
map_entry_t map_next(map_iter_t *iter);

NODISCARD
bool map_has_next(map_iter_t *iter);

void map_reset(map_t *map);
