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
    size_t size;  ///< the total number of allocated elements
    size_t used;  ///< the number of elements in use
    void *data[]; ///< the data
} vector_t;

/**
 * @brief release a vector
 *
 * @note doesnt release the data it references.
 *
 * @param vector the vector to release
 */
void vector_delete(vector_t *vector);

/**
 * @brief create a new vector.
 * 
 * creates a vector with a length of 0.
 *
 * @param size the initial amount of allocated memory
 * @return a new vector
 */
vector_t *vector_new(size_t size);

/**
 * @brief create a new vector with a specified length
 *
 * creates a new vector with a specified length.
 * these elements should be filled in manually by the caller.
 *
 * @param len the initial size of the vector
 * @return a new vector
 */
vector_t *vector_of(size_t len);

/**
 * @brief create a vector with a single item
 *
 * @param value the initial element
 * @return the new vector
 */
vector_t *vector_init(void *value);

/**
 * @brief add an element to the end of the vector
 * 
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
 * @brief set an element in a vector
 *
 * set an element in a vector by index.
 * index must not be out of range.
 *
 * @param vector the vector to set in
 * @param index the index to place the value
 * @param value the value to place
 */
void vector_set(vector_t *vector, size_t index, void *value);

/**
 * @brief read from a vector by index.
 * 
 * get an element from a vector by index.
 * index must not be out of range.
 *
 * @param vector the vector to get from
 * @param index the index to query
 * @return the value at index
 */
void *vector_get(const vector_t *vector, size_t index);

/**
 * @brief get a vectors last element
 * 
 * get the last element from a vector.
 * calling on an empty vector is invalid.
 *
 * @param vector the vector to get from
 * @return the value of the last element
 */
void *vector_tail(const vector_t *vector);

/**
 * @brief get vector length
 * 
 * get the length of a vector
 *
 * @param vector the vector to get the length of
 * @return the active size of the vector
 */
size_t vector_len(const vector_t *vector);

/**
 * @brief join two vectors
 *
 * join two vectors together into a new vector.
 *
 * @param lhs the left vector
 * @param rhs the right vector
 * @return the new vector
 */
vector_t *vector_join(const vector_t *lhs, const vector_t *rhs);

/**
 * @brief find an element in a vector
 * 
 * searches by pointer equality.
 *
 * @param vector the vector to search
 * @param element the element to search for
 * @return the index of the element of @a SIZE_MAX if not found
 */
size_t vector_find(vector_t *vector, const void *element);

/**
 * @brief reset the length of a vector
 * 
 * @param vec the vector to clear
 */
void vector_reset(vector_t *vec);
