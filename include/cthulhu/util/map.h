#pragma once

#include "vector.h"
#include <stddef.h>

/**
 * a bucket in a hashmap
 */
typedef struct bucket_t {
    FIELD_STR 
    const char *key; ///< can actually be any pointer but we keep it as a const char* for convenience
    
    void *value; ///< any pointer value
    struct bucket_t *next; ///< the next bucket in the chain
} bucket_t;

/**
 * a hashmap
 * 
 * freeing the map will not free the keys or the values.
 * these need to be freed beforehand by the owner of the container.
 */
typedef STRUCT_SIZE(sizeof(map_t) + size * sizeof(bucket_t)) struct {
    FIELD_RANGE(>, 0)
    size_t size; ///< the number of buckets in the toplevel
    
    FIELD_SIZE(size)
    bucket_t data[]; ///< the buckets
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
NODISCARD RET_NOTNULL
map_t *map_new(IN_RANGE(>, 0) size_t size);

/**
 * create a map with an optimal number of buckets 
 * for a given size
 * 
 * @param size the estimated number of elements to store
 * 
 * @return a new map
 */
NODISCARD RET_NOTNULL
map_t *optimal_map(size_t size);

/**
 * set or overwrite a value in a map
 * 
 * @param map the map to set the value in
 * @param key the key to set the value for
 * @param value the value to set
 */
void map_set(IN_NOTNULL map_t *map, IN_STR const char *key, void *value) HOT NOTNULL(1, 2);

/**
 * get a value from a map
 * 
 * @param map the map to get the value from
 * @param key the key to get the value for
 * 
 * @return the value for the key or NULL if the key is not found
 */
NODISCARD RET_NOTNULL
void *map_get(IN_NOTNULL map_t *map, IN_STR const char *key) HOT CONSTFN NONULL;

/**
 * @brief get a value from a map or a default value if the key is not found
 * 
 * @param map the map to get the value from
 * @param key the key to get the value for
 * @param other the default value to return if the key is not found
 * 
 * @return the value for the key or the default value if the key is not found
 */
NODISCARD
void *map_get_default(IN_NOTNULL map_t *map, IN_STR const char *key, void *other) HOT CONSTFN NOTNULL(1, 2);

/**
 * @brief set a field using a raw pointer rather than a string key
 * 
 * @param map the map to set the value in
 * @param key the key to set the value for
 * @param value the value to set
 */
void map_set_ptr(IN_NOTNULL map_t *map, IN_NOTNULL const void *key, void *value) HOT NOTNULL(1, 2);

/**
 * @brief get a field from a raw pointer rather than a string key
 * 
 * @param map the map to get the value from
 * @param key the key to get the value for
 * 
 * @return the value for the key or NULL if the key is not found
 */
NODISCARD
void *map_get_ptr(IN_NOTNULL map_t *map, IN_NOTNULL const void *key) HOT NONULL;

/**
 * @brief get a field from a raw pointer rather than a string key or a default value if the key is not found
 * 
 * @param map the map to get the value from
 * @param key the key to get the value for
 * @param other the default value to return if the key is not found
 * 
 * @return the value for the key or the default value if the key is not found
 */
NODISCARD
void *map_get_ptr_default(map_t *map, IN_NOTNULL const void *key, void *other) HOT NOTNULL(1, 2);

/**
 * @brief collect all the values in a map into a vector
 * 
 * @param map the map to collect the values from
 * 
 * @return a vector containing all the values
 */
NODISCARD RET_NOTNULL
vector_t *map_values(IN_NOTNULL map_t *map) NONULL ALLOC(vector_delete);
