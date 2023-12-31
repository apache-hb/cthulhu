#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stddef.h>

BEGIN_API

/// @defgroup Vector Generic vector
/// @ingroup Standard
/// @brief Generic vector of pointers
/// @{

typedef struct arena_t arena_t;

/**
 * a vector of non-owning pointers
 *
 * all elements in the vector must fit into a pointer each.
 * freeing the vector does not free the data it references internally.
 * only the vector itself is freed.
 */
typedef struct vector_t vector_t;

/**
 * @brief release a vector
 *
 * @note doesnt release the data it references.
 *
 * @param vector the vector to release
 */
void vector_delete(OUT_PTR_INVALID vector_t *vector);

NODISCARD
vector_t *vector_new_arena(size_t size, IN_NOTNULL arena_t *arena);

NODISCARD
vector_t *vector_of_arena(size_t len, IN_NOTNULL arena_t *arena);

NODISCARD
vector_t *vector_init_arena(void *value, IN_NOTNULL arena_t *arena);

/**
 * @brief create a new vector.
 *
 * creates a vector with a length of 0.
 *
 * @param size the initial amount of allocated memory
 * @return a new vector
 */
NODISCARD
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
NODISCARD
vector_t *vector_of(size_t len);

/**
 * @brief create a vector with a single item
 *
 * @param value the initial element
 * @return the new vector
 */
NODISCARD
vector_t *vector_init(void *value);

NODISCARD
vector_t *vector_clone(IN_NOTNULL vector_t *vector);

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
void vector_push(IN_NOTNULL vector_t **vector, void *value);

/**
 * @brief remove the last element from a vector. invalid on empty vectors
 *
 * @param vector the vector to drop an item from
 */
void vector_drop(IN_NOTNULL vector_t *vector);

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
void vector_set(
    IN_NOTNULL vector_t *vector,
    size_t index,
    void *value
);

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
NODISCARD CONSTFN
void *vector_get(
    IN_NOTNULL const vector_t *vector,
    size_t index
);

/**
 * @brief get a vectors last element
 *
 * get the last element from a vector.
 * calling on an empty vector is invalid.
 *
 * @param vector the vector to get from
 * @return the value of the last element
 */
NODISCARD CONSTFN
void *vector_tail(IN_NOTNULL const vector_t *vector);

/**
 * @brief get vector length
 *
 * get the length of a vector
 *
 * @param vector the vector to get the length of
 * @return the active size of the vector
 */
NODISCARD CONSTFN
size_t vector_len(IN_NOTNULL const vector_t *vector);

/**
 * @brief join two vectors
 *
 * join two vectors together into a new vector.
 *
 * @param lhs the left vector
 * @param rhs the right vector
 * @return the new vector
 */
NODISCARD
vector_t *vector_merge(IN_NOTNULL const vector_t *lhs, IN_NOTNULL const vector_t *rhs);

/**
 * @brief append a vector to the end of another
 *
 * @param vector the vector to append to
 * @param other the vector to append
 */
void vector_append(IN_NOTNULL vector_t **vector, IN_NOTNULL const vector_t *other);

NODISCARD
vector_t *vector_join(IN_NOTNULL vector_t *vectors);

/**
 * @brief find an element in a vector
 *
 * searches by pointer equality.
 *
 * @param vector the vector to search
 * @param element the element to search for
 * @return the index of the element of @a SIZE_MAX if not found
 */
RET_INSPECT CONSTFN
size_t vector_find(IN_NOTNULL vector_t *vector, const void *element);

/**
 * @brief reset the length of a vector
 *
 * @param vec the vector to clear
 */
void vector_reset(IN_NOTNULL vector_t *vec);

void **vector_data(IN_NOTNULL vector_t *vec);

/// @}

END_API
