#pragma once

#include "macros.h"
#include <stddef.h>

typedef void*(*vector_apply_t)(void *value);

/**
 * a vector of non-owning pointers
 * 
 * all elements in the vector must fit into a pointer each.
 * freeing the vector does not free the data it references internally.
 * only the vector itself is freed.
 */
typedef struct {
    size_t size; ///< the total number of allocated elements
    size_t used; ///< the number of elements in use
    FIELD_SIZE(size) FIELD_RANGE(0, used) void *data[]; ///< the data
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
NODISCARD vector_t *vector_new(size_t size) ALLOC(vector_delete);

/**
 * create a new vector with a specified size
 * 
 * @param len the initial size of the vector
 * @return a new vector
 */
NODISCARD vector_t *vector_of(size_t len) ALLOC(vector_delete);

/**
 * create a vector with a single item
 * 
 * @param value the initial element
 * @return the new vector
 */
NODISCARD vector_t *vector_init(void *value) ALLOC(vector_delete);

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
NODISCARD void *vector_pop(vector_t *vector) NONULL;

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
NODISCARD void *vector_get(const vector_t *vector, size_t index) CONSTFN NOTNULL(1);

/**
 * get the last element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to get from
 * @return the value of the last element
 */
NODISCARD void *vector_tail(const vector_t *vector) CONSTFN NONULL;

/**
 * @brief get the first element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to get from
 * @return void* the value of the first element
 */
NODISCARD void *vector_head(const vector_t *vector) CONSTFN NONULL;

/**
 * get the contents pointer of a vector.
 * 
 * @param vector the vector to get the contents of
 * @return the contents pointer
 */
NODISCARD void **vector_data(vector_t *vector) CONSTFN NONULL;

/**
 * get the length of a vector
 * 
 * @param vector the vector to get the length of
 * @return the active size of the vector
 */
NODISCARD size_t vector_len(const vector_t *vector) CONSTFN NONULL;

/**
 * join two vectors together into a new vector.
 * 
 * @param lhs the left vector
 * @param rhs the right vector
 * @return the new vector
 */
NODISCARD vector_t *vector_join(const vector_t *lhs, const vector_t *rhs) NONULL ALLOC(vector_delete);

/**
 * @brief create a new vector given a front and back index
 * 
 * @param vector the vector to take a slice of
 * @param start the first index to include
 * @param end  the last index to include
 * @return vector_t* the new vector
 */
NODISCARD vector_t *vector_slice(
    vector_t *vector, 
    IN_RANGE(0, vector->used) size_t start, 
    IN_RANGE(start, vector->used) size_t end
) NONULL;

NODISCARD size_t vector_find(vector_t *vector, const void *element) CONSTFN NOTNULL(1);

void vector_reset(vector_t *vec) NONULL;

/**
 * @brief collect a vector of vectors into a single vector
 * 
 * @param vectors the vectors
 * @return vector_t* the merged vector
 */
NODISCARD vector_t *vector_collect(vector_t *vectors) NONULL ALLOC(vector_delete);

/**
 * return a new vector after applying a function to all elements
 * 
 * @param vector the vector to map from
 * @param func the function to apply
 * @return the new vector
 */
NODISCARD vector_t *vector_map(const vector_t *vector, vector_apply_t func) NONULL ALLOC(vector_delete);

#define VECTOR_MAP(vector, func) vector_map(vector, (vector_apply_t)func)
