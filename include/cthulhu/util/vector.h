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
typedef struct STRUCT_SIZE(sizeof(vector_t) + size * sizeof(void*)) {
    FIELD_RANGE(>=, used)
    size_t size; ///< the total number of allocated elements
    
    FIELD_RANGE(<=, size)
    size_t used; ///< the number of elements in use
    
    FIELD_SIZE(size) 
    void *data[]; ///< the data
} vector_t;

/**
 * release a vector
 * 
 * @param vector the vector to release
 */
void vector_delete(IN_NOTNULL vector_t *vector) NONULL;

/**
 * create a new vector.
 * 
 * @param size the initial amount of allocated memory
 * @return a new vector
 */
RESULT(return->size >= size)
RESULT(return->used == 0)
NODISCARD RET_VALID
vector_t *vector_new(size_t size) ALLOC(vector_delete);

/**
 * create a new vector with a specified size
 * 
 * @param len the initial size of the vector
 * @return a new vector
 */
RESULT(return->size >= len)
RESULT(return->used == len)
NODISCARD RET_VALID
vector_t *vector_of(size_t len) ALLOC(vector_delete);

/**
 * create a vector with a single item
 * 
 * @param value the initial element
 * @return the new vector
 */
RESULT(return->size >= 1)
RESULT(return->used == 1)
NODISCARD RET_VALID
vector_t *vector_init(void *value) ALLOC(vector_delete);

/**
 * push an element onto the end of a vector
 * 
 * @param vector a pointer to a vector returned by vector_new.
 *               the pointer may be modified by this function.
 * 
 * @param value the value to push onto the vector
 */
void vector_push(IN_NOTNULL vector_t **vector, void *value) NOTNULL(1);

/**
 * @brief remove the last element from a vector. invalid on empty vectors
 * 
 * @param vector the vector to drop an item from
 */
ALWAYS(vector->used > 0)
void vector_drop(IN_NOTNULL vector_t *vector) NONULL;

/**
 * set an element in a vector by index.
 * index must not be out of range.
 * 
 * @param vector the vector to set in
 * @param index the index to place the value
 * @param value the value to place
 */
void vector_set(IN_NOTNULL vector_t *vector, IN_RANGE(<=, vector->used) size_t index, void *value) NOTNULL(1);

/**
 * get an element from a vector by index.
 * index must not be out of range.
 * 
 * @param vector the vector to get from
 * @param index the index to query
 * @return the value at index
 */
NODISCARD
void *vector_get(IN_NOTNULL const vector_t *vector, IN_RANGE(<=, vector->used) size_t index) CONSTFN NOTNULL(1);

/**
 * get the last element from a vector.
 * calling on an empty vector is invalid.
 * 
 * @param vector the vector to get from
 * @return the value of the last element
 */
ALWAYS(vector->used > 0)
NODISCARD
void *vector_tail(IN_NOTNULL const vector_t *vector) CONSTFN NONULL;

/**
 * get the length of a vector
 * 
 * @param vector the vector to get the length of
 * @return the active size of the vector
 */
RESULT(return <= vector->size)
NODISCARD
size_t vector_len(IN_NOTNULL const vector_t *vector) CONSTFN NONULL;

/**
 * join two vectors together into a new vector.
 * 
 * @param lhs the left vector
 * @param rhs the right vector
 * @return the new vector
 */
RESULT(return->used == lhs->used + rhs->used)
NODISCARD RET_VALID
vector_t *vector_join(IN_NOTNULL const vector_t *lhs, IN_NOTNULL const vector_t *rhs) NONULL ALLOC(vector_delete);

RESULT(return <= vector->used)
NODISCARD
size_t vector_find(IN_NOTNULL vector_t *vector, const void *element) CONSTFN NOTNULL(1);

RESULT(vec->used == 0)
void vector_reset(IN_NOTNULL vector_t *vec) NONULL;
