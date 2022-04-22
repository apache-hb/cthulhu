#pragma once

#include "macros.h"
#include <stddef.h>

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
    void *data[]; ///< the data
} vector_t;

/**
 * release a vector
 * 
 * @param vector the vector to release
 */
void vector_delete(vector_t *vector);

/**
 * create a new vector.
 * 
 * @param size the initial amount of allocated memory
 * @return a new vector
 */
vector_t *vector_new(size_t size);

/**
 * create a new vector with a specified size
 * 
 * @param len the initial size of the vector
 * @return a new vector
 */
vector_t *vector_of(size_t len);

/**
 * create a vector with a single item
 * 
 * @param value the initial element
 * @return the new vector
 */
vector_t *vector_init(void *value);

/**
 * push an element onto the end of a vector
 * 
 * @param vector a pointer to a vector returned by vector_new.
 *               the pointer may be modified by this function.
 * 
 * @param value the value to push onto the vector
 */
void vector_push(vector_t **vector, void *value);

/**
 * @brief remove the last element from a vector. invalid on empty vectors
 * 
 * @param vector the vector to drop an item from
 */
void vector_drop(vector_t *vector);

/**
 * set an element in a vector by index.
 * index must not be out of range.
 * 
 * @param vector the vector to set in
 * @param index the index to place the value
 * @param value the value to place
 */
void vector_set(vector_t *vector, size_t index, void *value);

/**
 * get an element from a vector by index.
 * index must not be out of range.
 * 
 * @param vector the vector to get from
 * @param index the index to query
 * @return the value at index
 */
void *vector_get(const vector_t *vector, size_t index);

/**
 * get the last element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to get from
 * @return the value of the last element
 */
void *vector_tail(const vector_t *vector);

/**
 * get the length of a vector
 * 
 * @param vector the vector to get the length of
 * @return the active size of the vector
 */
size_t vector_len(const vector_t *vector);

/**
 * join two vectors together into a new vector.
 * 
 * @param lhs the left vector
 * @param rhs the right vector
 * @return the new vector
 */
vector_t *vector_join(const vector_t *lhs, const vector_t *rhs);

size_t vector_find(vector_t *vector, const void *element);

void vector_reset(vector_t *vec);
