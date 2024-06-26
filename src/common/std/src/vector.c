// SPDX-License-Identifier: LGPL-3.0-only

#include "std/vector.h"
#include "arena/arena.h"
#include "base/panic.h"
#include "base/util.h"

/**
 * a vector of non-owning pointers
 *
 * all elements in the vector must fit into a pointer each.
 * freeing the vector does not free the data it references internally.
 * only the vector itself is freed.
 */
typedef struct vector_t
{
    arena_t *arena;

    size_t size;                   ///< the total number of allocated elements
    size_t used;                   ///< the number of elements in use
    STA_FIELD_SIZE(size) void *data[]; ///< the data
} vector_t;

vector_t gEmptyVector = { NULL, 0, 0 };
const vector_t kEmptyVector = { NULL, 0, 0 };

// get the size of the vector struct given a number of elements
static size_t vector_typesize(size_t size)
{
    return sizeof(vector_t) + (size * sizeof(void *));
}

#define CHECK_VECTOR(vector) \
    CTASSERT((vector) != NULL); \
    CTASSERT(*(vector) != NULL);

#define VEC (*vector)

static void vector_ensure(vector_t **vector, size_t size)
{
    if (size >= VEC->size)
    {
        size_t resize = (size + 1) * 2;
        VEC = arena_realloc(VEC, vector_typesize(resize), vector_typesize(size), VEC->arena);
        VEC->size = resize;
    }
}

static vector_t *vector_init_inner(size_t size, size_t used, arena_t *arena)
{
    vector_t *vector = ARENA_MALLOC(vector_typesize(size), "vector", NULL, arena);

    vector->arena = arena;
    vector->size = size;
    vector->used = used;

    return vector;
}

// vector public api

STA_DECL
vector_t *vector_new(size_t size, arena_t *arena)
{
    CTASSERT(arena != NULL);

    return vector_init_inner(size, 0, arena);
}

STA_DECL
vector_t *vector_of(size_t len, arena_t *arena)
{
    CTASSERT(arena != NULL);

    return vector_init_inner(len, len, arena);
}

STA_DECL
vector_t *vector_init(void *value, arena_t *arena)
{
    CTASSERT(arena != NULL);

    vector_t *vector = vector_of(1, arena);
    vector_set(vector, 0, value);
    return vector;
}

STA_DECL
vector_t *vector_clone(vector_t *vector)
{
    CTASSERT(vector != NULL);

    size_t len = vector_len(vector);
    vector_t *clone = vector_of(len, vector->arena);
    ctu_memcpy(clone->data, vector->data, len * sizeof(void *));
    return clone;
}

STA_DECL
void vector_delete(vector_t *vector)
{
    CTASSERT(vector != NULL);

    arena_free(vector, vector_typesize(vector->size), vector->arena);
}

STA_DECL
void vector_push(vector_t **vector, void *value)
{
    CHECK_VECTOR(vector);

    vector_ensure(vector, VEC->used + 1);
    VEC->data[VEC->used++] = value;
}

STA_DECL
void vector_drop(vector_t *vector)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > 0);
    vector->used -= 1;
}

STA_DECL
void vector_set(vector_t *vector, size_t index, void *value)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > index);

    vector->data[index] = value;
}

STA_DECL
void *vector_get(const vector_t *vector, size_t index)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > index);

    return vector->data[index];
}

STA_DECL
void *vector_tail(const vector_t *vector)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > 0);

    return vector->data[vector->used - 1];
}

STA_DECL
size_t vector_len(const vector_t *vector)
{
    CTASSERT(vector != NULL);

    return vector->used;
}

STA_DECL
size_t vector_find(vector_t *vector, const void *element)
{
    CTASSERT(vector != NULL);

    for (size_t i = 0; i < vector_len(vector); i++)
    {
        if (vector_get(vector, i) == element)
        {
            return i;
        }
    }

    return SIZE_MAX;
}

STA_DECL
void vector_append(vector_t **vector, const vector_t *other)
{
    CHECK_VECTOR(vector);

    size_t len = vector_len(other);

    vector_ensure(vector, VEC->used + len);
    ctu_memcpy(VEC->data + VEC->used, other->data, len * sizeof(void *));
    VEC->used += len;
}

STA_DECL
void vector_reset(vector_t *vec)
{
    CTASSERT(vec != NULL);

    vec->used = 0;
}

STA_DECL
void **vector_data(vector_t *vec)
{
    CTASSERT(vec != NULL);

    return vec->data;
}
