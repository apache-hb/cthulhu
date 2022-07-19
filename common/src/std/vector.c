#include "std/vector.h"
#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include <stdint.h>

/**
 * a vector of non-owning pointers
 *
 * all elements in the vector must fit into a pointer each.
 * freeing the vector does not free the data it references internally.
 * only the vector itself is freed.
 */
typedef struct vector_t
{
    alloc_t *alloc;

    size_t size;                   ///< the total number of allocated elements
    size_t used;                   ///< the number of elements in use
    FIELD_SIZE(size) void *data[]; ///< the data
} vector_t;

NODISCARD
static size_t vector_size(size_t size)
{
    return sizeof(vector_t) + (size * sizeof(void *));
}

#define VEC (*vector)

static void vector_ensure(vector_t **vector, size_t size)
{
    if (size >= VEC->size)
    {
        size_t resize = (size + 1) * 2;
        VEC = arena_realloc(VEC->alloc, VEC, vector_size(resize), vector_size(VEC->size));
        VEC->size = resize;
    }
}

// vector public api

USE_DECL
vector_t *vector_new2(size_t size, alloc_t *alloc, const char *name)
{
    vector_t *vector = arena_malloc(alloc, vector_size(size), name);

    vector->alloc = alloc;
    vector->size = size;
    vector->used = 0;

    return vector;
}

USE_DECL
vector_t *vector_new(size_t size)
{
    return vector_new2(size, &globalAlloc, "vector-new");
}

USE_DECL
vector_t *vector_of(size_t len)
{
    vector_t *vector = vector_new(len);
    vector->used = len;
    return vector;
}

USE_DECL
vector_t *vector_init(void *value)
{
    vector_t *vector = vector_of(1);
    vector_set(vector, 0, value);
    return vector;
}

void vector_delete(vector_t *vector)
{
    arena_free(vector->alloc, vector, vector_size(vector->size));
}

void vector_push(vector_t **vector, void *value)
{
    vector_ensure(vector, VEC->used + 1);
    VEC->data[VEC->used++] = value;
}

void vector_drop(vector_t *vector)
{
    CTASSERT(vector_len(vector) > 0);
    vector->used -= 1;
}

void vector_set(vector_t *vector, size_t index, void *value)
{
    vector->data[index] = value;
}

USE_DECL
void *vector_get(const vector_t *vector, size_t index)
{
    return vector->data[index];
}

USE_DECL
void *vector_tail(const vector_t *vector)
{
    return vector->data[vector->used - 1];
}

USE_DECL
size_t vector_len(const vector_t *vector)
{
    return vector->used;
}

USE_DECL
size_t vector_find(vector_t *vector, const void *element)
{
    for (size_t i = 0; i < vector_len(vector); i++)
    {
        if (vector_get(vector, i) == element)
        {
            return i;
        }
    }

    return SIZE_MAX;
}

USE_DECL
vector_t *vector_merge(const vector_t *lhs, const vector_t *rhs)
{
    size_t lhsLength = vector_len(lhs);
    size_t rhsLength = vector_len(rhs);

    size_t len = lhsLength + rhsLength;

    vector_t *out = vector_new(len);
    out->used = len;

    for (size_t i = 0; i < lhsLength; i++)
    {
        vector_set(out, i, vector_get(lhs, i));
    }

    for (size_t i = 0; i < rhsLength; i++)
    {
        vector_set(out, lhsLength + i, vector_get(rhs, i));
    }

    return out;
}

USE_DECL
vector_t *vector_join(vector_t *vectors)
{
    size_t totalLength = 0;
    size_t vecLength = vector_len(vectors);

    for (size_t i = 0; i < vecLength; i++)
    {
        totalLength += vector_len(vector_get(vectors, i));
    }

    vector_t *result = vector_of(totalLength);
    size_t offset = 0;

    for (size_t i = 0; i < vecLength; i++)
    {
        vector_t *vector = vector_get(vectors, i);
        size_t length = vector_len(vector);

        for (size_t j = 0; j < length; j++)
        {
            vector_set(result, offset++, vector_get(vector, j));
        }
    }

    return result;
}

void vector_reset(vector_t *vec)
{
    vec->used = 0;
}
