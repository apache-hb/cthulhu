#pragma once

#include <stdbool.h>

#include "sizes.h"
#include "macros.h"

/**
 * deprecated memory managment functions
 * 
 * arenas should be used in place of these for performance reasons
 */
void ctu_free(void *ptr) NONULL;
void *ctu_malloc(size_t size) ALLOC(ctu_free);
void *ctu_realloc(void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);
char *ctu_strdup(const char *str) NONULL ALLOC(ctu_free);
char *ctu_strndup(const char *str, size_t len) NONULL ALLOC(ctu_free);
void *ctu_memdup(const void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);

void init_gmp(void);

/**
 * a hashmap of strings to weak pointers
 * 
 * freeing the map will not free the keys or the values.
 * these need to be freed beforehand by the owner of the container.
 */
typedef struct bucket_t {
    WEAK NULLABLE const char *key;
    WEAK NULLABLE void *value;
    OWNED NULLABLE struct bucket_t *next;
} bucket_t;

typedef struct {
    size_t size;
    bucket_t data[];
} map_t;

typedef void(*map_apply_t)(void *user, void *value);
typedef bool(*map_collect_t)(void *value);
typedef void*(*vector_apply_t)(void *value);

/**
 * create a new map
 * 
 * @param size the number of toplevel buckets used.
 *             increasing this value reduces hash collisions
 *             but increases memory usage and initialization time.
 * 
 * @return a new map
 */
OWNED map_t *map_new(map_size_t size);

/**
 * create a map with an optimal number of buckets 
 * for a given size
 * 
 * @param size the estimated number of elements to store
 * 
 * @return a new map
 */
OWNED map_t *optimal_map(size_t size);

/**
 * get a value from a map
 * 
 * @param map the map to get the value from
 * @param key the key to get the value for
 * 
 * @return the value for the key or NULL if the key is not found
 */
WEAK void *map_get(WEAK map_t *map, const char *key) HOT CONSTFN NONULL;

/**
 * set or overwrite a value in a map
 * 
 * @param map the map to set the value in
 * @param key the key to set the value for
 * @param value the value to set
 */
void map_set(WEAK map_t *map, const char *key, WEAK void *value) HOT NOTNULL(1, 2);

/**
 * apply a function to all values in a map
 * 
 * @param map the map to apply the function to
 * @param user user data passed into func
 * @param func the function to apply
 */
void map_apply(WEAK map_t *map, WEAK void *user, map_apply_t func) NOTNULL(1, 3);

/**
 * a vector of non-owning pointers
 * 
 * all elements in the vector must fit into a pointer each.
 * freeing the vector does not free the data it references internally.
 * only the vector itself is freed.
 */
typedef struct {
    size_t size;
    size_t used;
    void *data[];
} vector_t;

/**
 * release a vector
 * 
 * @param vector the vector to release
 */
void vector_delete(OWNED vector_t *vector) NONULL;

/**
 * create a new vector.
 * 
 * @param size the initial amount of allocated memory
 * @return a new vector
 */
OWNED vector_t *vector_new(size_t size) ALLOC(vector_delete);

/**
 * create a new vector with a specified size
 * 
 * @param len the initial size of the vector
 * @return a new vector
 */
OWNED vector_t *vector_of(size_t len) ALLOC(vector_delete);

/**
 * create a vector with a single item
 * 
 * @param value the initial element
 * @return the new vector
 */
OWNED vector_t *vector_init(WEAK void *value) ALLOC(vector_delete);

/**
 * push an element onto the end of a vector
 * 
 * @param vector a pointer to a vector returned by vector_new.
 *               the pointer may be modified by this function.
 * 
 * @param value the value to push onto the vector
 */
void vector_push(WEAK vector_t **vector, WEAK void *value) NOTNULL(1);

/**
 * pop the last element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to take from
 * @return the value of the last element
 */
OWNED void *vector_pop(vector_t *vector) NONULL;

/**
 * set an element in a vector by index.
 * index must not be out of range.
 * 
 * @param vector the vector to set in
 * @param index the index to place the value
 * @param value the value to place
 */
void vector_set(vector_t *vector, size_t index, void *value) NOTNULL(1);

/**
 * get an element from a vector by index.
 * index must not be out of range.
 * 
 * @param vector the vector to get from
 * @param index the index to query
 * @return the value at index
 */
WEAK void *vector_get(WEAK const vector_t *vector, size_t index) CONSTFN NOTNULL(1);

/**
 * get the last element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to get from
 * @return the value of the last element
 */
WEAK void *vector_tail(WEAK const vector_t *vector) CONSTFN NONULL;

/**
 * get the contents pointer of a vector.
 * 
 * @param vector the vector to get the contents of
 * @return the contents pointer
 */
WEAK void **vector_data(vector_t *vector) CONSTFN NONULL;

/**
 * get the length of a vector
 * 
 * @param vector the vector to get the length of
 * @return the active size of the vector
 */
size_t vector_len(const vector_t *vector) CONSTFN NONULL;

/**
 * join two vectors together into a new vector.
 * 
 * @param lhs the left vector
 * @param rhs the right vector
 * @return the new vector
 */
OWNED vector_t *vector_join(WEAK const vector_t *lhs, WEAK const vector_t *rhs) NONULL ALLOC(vector_delete);

/**
 * return a new vector after applying a function to all elements
 * 
 * @param vector the vector to map from
 * @param func the function to apply
 * @return the new vector
 */
OWNED vector_t *vector_map(WEAK const vector_t *vector, vector_apply_t func) NONULL ALLOC(vector_delete);

OWNED vector_t *map_collect(WEAK map_t *map, map_collect_t filter) NONULL ALLOC(vector_delete);
OWNED vector_t *map_values(WEAK map_t *map) NONULL ALLOC(vector_delete);

#define MAP_APPLY(map, user, func) map_apply(map, user, (map_apply_t)func)
#define MAP_COLLECT(map, filter) map_collect(map, (map_collect_t)filter)
#define VECTOR_MAP(vector, func) vector_map(vector, (vector_apply_t)func)
