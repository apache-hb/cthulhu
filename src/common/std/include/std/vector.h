// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_std_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/types.h"

CT_BEGIN_API

typedef struct arena_t arena_t;

/// @defgroup vector Generic vector
/// @ingroup standard
/// @brief Generic vector
/// @{

/// @brief a generic vector of pointers
typedef struct vector_t vector_t;

/// @brief a global empty vector
/// used to avoid allocating alot of empty vectors
///
/// holding off on making it const until all the parsers are updated to handle it
/// @warning doing anything with this vector aside from getting its length is invalid
CT_STD_API extern vector_t gEmptyVector;

/// @brief a global empty vector
/// used to avoid allocating alot of empty vectors
///
/// @warning doing anything with this vector aside from getting its length is invalid
CT_STD_API extern const vector_t kEmptyVector;

/// @brief destroy a vector
/// @note this only frees the vector itself, not the data it references
///
/// @param vector the vector to destroy
CT_STD_API void vector_delete(OUT_PTR_INVALID vector_t *vector);

/// @brief create a new vector with an initial capacity
///
/// @param size the initial capacity of the vector
/// @param arena the arena to allocate from
///
/// @return a new vector
CT_NODISCARD
CT_STD_API vector_t *vector_new(size_t size, IN_NOTNULL arena_t *arena);

/// @brief create a new vector with a specified length
/// @note these elements should be filled in manually by the caller using @a vector_set
///       @a vector_push is invalid until the vector is filled.
///
/// @param len the initial size of the vector
/// @param arena the arena to allocate from
///
/// @return a new vector
CT_NODISCARD
CT_STD_API vector_t *vector_of(size_t len, IN_NOTNULL arena_t *arena);

/// @brief create a new vector with a single initial value
///
/// this is equivalent to
/// @code{.c}
/// vector_t *vec = vector_of(1, arena);
/// vector_set(vec, 0, value);
/// @endcode
///
/// @param value the initial value of the vector
/// @param arena the arena to allocate from
///
/// @return a new vector
CT_NODISCARD
CT_STD_API vector_t *vector_init(void *value, IN_NOTNULL arena_t *arena);

/// @brief clone a vector
///
/// @param vector the vector to clone
///
/// @return a new vector with the same contents as @p vector
CT_NODISCARD
CT_STD_API vector_t *vector_clone(IN_NOTNULL vector_t *vector);

/// @brief push a value onto the end of a vector
///
/// @param vector the vector to push onto
/// @param value the value to push
CT_STD_API void vector_push(IN_NOTNULL vector_t **vector, void *value);

/// @brief pop a value from the end of a vector
/// @warning invalid on an empty vector
///
/// @param vector the vector to pop from
CT_STD_API void vector_drop(IN_NOTNULL vector_t *vector);

/// @brief set a value in a vector
/// @pre @p index < @a vector_len(vector)
///
/// @param vector the vector to set in
/// @param index the index to set
/// @param value the value to set
CT_STD_API void vector_set(
    IN_NOTNULL vector_t *vector,
    ctu_length_t index,
    void *value
);

/// @brief get a value from a vector
/// @pre @p index < @a vector_len(vector)
///
/// @param vector the vector to get from
/// @param index the index to get
///
/// @return the value at @p index
CT_NODISCARD CT_PUREFN
CT_STD_API void *vector_get(
    IN_NOTNULL const vector_t *vector,
    ctu_length_t index
);

/// @brief get the last element of a vector
/// @pre @a vector_len(vector) > 0
///
/// @param vector the vector to get the last element of
///
/// @return the last element of @p vector
CT_NODISCARD CT_PUREFN
CT_STD_API void *vector_tail(IN_NOTNULL const vector_t *vector);

/// @brief get the length of a vector
///
/// @param vector the vector to get the length of
///
/// @return the length of @p vector
CT_NODISCARD CT_PUREFN
CT_STD_API ctu_length_t vector_len(IN_NOTNULL const vector_t *vector);

/// @brief append the contents of one vector to another
/// this copies the contents of @p other into @p vector
///
/// @param vector the vector to append to
/// @param other the vector to append
CT_STD_API void vector_append(IN_NOTNULL vector_t **vector, IN_NOTNULL const vector_t *other);

/// @brief find an element in a vector
/// searches via pointer equality
///
/// @param vector the vector to search
/// @param element the element to search for
///
/// @return the index of @p element in @p vector or @a SIZE_MAX if not found
RET_INSPECT CT_PUREFN
CT_STD_API ctu_length_t vector_find(IN_NOTNULL vector_t *vector, const void *element);

/// @brief reset the contents of a vector
/// @warning this does not free the data the vector references
///
/// @param vec the vector to reset
CT_STD_API void vector_reset(IN_NOTNULL vector_t *vec);

/// @brief get the data of a vector
/// @warning this is only valid until the next modification of the vector
///
/// @param vec the vector to get the data of
///
/// @return the data of @p vec
RET_NOTNULL
CT_STD_API void **vector_data(IN_NOTNULL vector_t *vec);

/// @}

CT_END_API
