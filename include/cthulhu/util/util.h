#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "sizes.h"
#include "macros.h"

/**
 * memory managment functions
 * 
 * TODO: arenas will be faster
 */
void ctu_free(void *ptr) NONULL;
void *ctu_malloc(size_t size) ALLOC(ctu_free);
void *ctu_realloc(void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);
char *ctu_strdup(const char *str) NONULL ALLOC(ctu_free);
char *ctu_strndup(const char *str, size_t len) NONULL ALLOC(ctu_free);
char *ctu_strdup_len(const char *str, size_t *len) NONULL ALLOC(ctu_free);
void *ctu_memdup(const void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);

#if ENABLE_TUNING
typedef struct {
    size_t mallocs; // calls to malloc
    size_t reallocs; // calls to realloc
    size_t frees; // calls to free

    size_t current; // current memory allocated
    size_t peak; // peak memory allocated
} counters_t;
counters_t get_counters(void);
counters_t reset_counters(void);
#endif

/**
 * @brief init gmp with our own allocation functions
 */
void init_gmp(void);

/**
 * @brief box a value onto the heap from the stack
 * 
 * @param ptr the value to box
 * @param size the size of the value
 * 
 * @return the boxed value
 * 
 * @see #MACRO(box) should be used to use this
 */
void *ctu_box(const void *ptr, size_t size) NOTNULL(1) ALLOC(ctu_free);
#define BOX(name) ctu_box(&name, sizeof(name))

/**
 * a hashset
 */

typedef struct item_t {
    const char *key;
    struct item_t *next;
} item_t;

typedef struct {
    size_t size;
    item_t items[];
} set_t;

set_t *set_new(size_t size);
void set_delete(set_t *set);

const char* set_add(set_t *set, const char *key);
bool set_contains(set_t *set, const char *key);

typedef struct bucket_t {
    const char *key; /// can actually be any pointer but we keep it as a const char* for convenience
    void *value; /// any pointer value
    struct bucket_t *next; /// the next bucket in the chain
} bucket_t;

/**
 * a hashmap
 * 
 * freeing the map will not free the keys or the values.
 * these need to be freed beforehand by the owner of the container.
 */
typedef struct {
    size_t size; /// the number of buckets in the toplevel
    bucket_t data[]; /// the buckets
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
map_t *map_new(map_size_t size);

/**
 * create a map with an optimal number of buckets 
 * for a given size
 * 
 * @param size the estimated number of elements to store
 * 
 * @return a new map
 */
map_t *optimal_map(size_t size);

/**
 * get a value from a map
 * 
 * @param map the map to get the value from
 * @param key the key to get the value for
 * 
 * @return the value for the key or NULL if the key is not found
 */
void *map_get(map_t *map, const char *key) HOT CONSTFN NONULL;

void *map_get_default(map_t *map, const char *key, void *other) HOT CONSTFN NOTNULL(1, 2);

/**
 * set or overwrite a value in a map
 * 
 * @param map the map to set the value in
 * @param key the key to set the value for
 * @param value the value to set
 */
void map_set(map_t *map, const char *key, void *value) HOT NOTNULL(1, 2);

/**
 * apply a function to all values in a map
 * 
 * @param map the map to apply the function to
 * @param user user data passed into func
 * @param func the function to apply
 */
void map_apply(map_t *map, void *user, map_apply_t func) NOTNULL(1, 3);

/**
 * @brief set a field using a raw pointer rather than a string key
 * 
 * @param map the map to set the value in
 * @param key the key to set the value for
 * @param value the value to set
 */
void map_set_ptr(map_t *map, const void *key, void *value) HOT NOTNULL(1, 2);

/**
 * @brief get a field from a raw pointer rather than a string key
 * 
 * @param map the map to get the value from
 * @param key the key to get the value for
 * 
 * @return the value for the key or NULL if the key is not found
 */
void *map_get_ptr(map_t *map, const void *key) HOT NONULL;

void *map_get_ptr_default(map_t *map, const void *key, void *other) HOT NOTNULL(1, 2);

/**
 * a vector of non-owning pointers
 * 
 * all elements in the vector must fit into a pointer each.
 * freeing the vector does not free the data it references internally.
 * only the vector itself is freed.
 */
typedef struct {
    size_t size; /// the total number of allocated elements
    size_t used; /// the number of elements in use
    void *data[]; /// the data
} vector_t;

/**
 * release a vector
 * 
 * @param vector the vector to release
 */
void vector_delete(vector_t *vector) NONULL;

/**
 * create a new vector.
 * 
 * @param size the initial amount of allocated memory
 * @return a new vector
 */
vector_t *vector_new(size_t size) ALLOC(vector_delete);

/**
 * create a new vector with a specified size
 * 
 * @param len the initial size of the vector
 * @return a new vector
 */
vector_t *vector_of(size_t len) ALLOC(vector_delete);

/**
 * create a vector with a single item
 * 
 * @param value the initial element
 * @return the new vector
 */
vector_t *vector_init(void *value) ALLOC(vector_delete);

/**
 * push an element onto the end of a vector
 * 
 * @param vector a pointer to a vector returned by vector_new.
 *               the pointer may be modified by this function.
 * 
 * @param value the value to push onto the vector
 */
void vector_push(vector_t **vector, void *value) NOTNULL(1);

/**
 * @brief remove the last element from a vector. invalid on empty vectors
 * 
 * @param vector the vector to drop an item from
 */
void vector_drop(vector_t **vector) NONULL;

/**
 * pop the last element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to take from
 * @return the value of the last element
 */
void *vector_pop(vector_t *vector) NONULL;

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
void *vector_get(const vector_t *vector, size_t index) CONSTFN NOTNULL(1);

/**
 * get the last element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to get from
 * @return the value of the last element
 */
void *vector_tail(const vector_t *vector) CONSTFN NONULL;

/**
 * @brief get the first element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to get from
 * @return void* the value of the first element
 */
void *vector_head(const vector_t *vector) CONSTFN NONULL;

/**
 * get the contents pointer of a vector.
 * 
 * @param vector the vector to get the contents of
 * @return the contents pointer
 */
void **vector_data(vector_t *vector) CONSTFN NONULL;

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
vector_t *vector_join(const vector_t *lhs, const vector_t *rhs) NONULL ALLOC(vector_delete);

/**
 * @brief create a new vector given a front and back index
 * 
 * @param vector the vector to take a slice of
 * @param start the first index to include
 * @param end  the last index to include
 * @return vector_t* the new vector
 */
vector_t *vector_slice(vector_t *vector, size_t start, size_t end) NONULL;

typedef bool(*vector_cmp_t)(const void *, const void *);

size_t vector_find(vector_t *vector, const void *element, vector_cmp_t cmp) CONSTFN NOTNULL(1);

void vector_reset(vector_t *vec) NONULL;

/**
 * @brief collect a vector of vectors into a single vector
 * 
 * @param vectors the vectors
 * @return vector_t* the merged vector
 */
vector_t *vector_collect(vector_t *vectors);

/**
 * return a new vector after applying a function to all elements
 * 
 * @param vector the vector to map from
 * @param func the function to apply
 * @return the new vector
 */
vector_t *vector_map(const vector_t *vector, vector_apply_t func) NONULL ALLOC(vector_delete);

vector_t *map_collect(map_t *map, map_collect_t filter) NONULL ALLOC(vector_delete);
vector_t *map_values(map_t *map) NONULL ALLOC(vector_delete);

#define MAP_APPLY(map, user, func) map_apply(map, user, (map_apply_t)func)
#define MAP_COLLECT(map, filter) map_collect(map, (map_collect_t)filter)
#define VECTOR_MAP(vector, func) vector_map(vector, (vector_apply_t)func)
