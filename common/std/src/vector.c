#include "std/vector.h"
#include "memory/memory.h"
#include "base/panic.h"
#include "core/macros.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * a vector of non-owning pointers
 *
 * all elements in the vector must fit into a pointer each.
 * freeing the vector does not free the data it references internally.
 * only the vector itself is freed.
 */
typedef struct vector_t
{
    size_t size;                   ///< the total number of allocated elements
    size_t used;                   ///< the number of elements in use
    FIELD_SIZE(size) void *data[]; ///< the data
} vector_t;

NODISCARD
static size_t vector_size(size_t size)
{
    return sizeof(vector_t) + (size * sizeof(void *));
}

#define CHECK_VECTOR(vector) \
    CTASSERT(vector != NULL); \
    CTASSERT(*vector != NULL);

#define VEC (*vector)

static void vector_ensure(vector_t **vector, size_t size)
{
    if (size >= VEC->size)
    {
        size_t resize = (size + 1) * 2;
        VEC = ctu_realloc(VEC, vector_size(resize));
        VEC->size = resize;
    }
}

static vector_t *vector_init_inner(size_t size, size_t used)
{
    vector_t *vector = MEM_ALLOC(vector_size(size), "vector", NULL);

    vector->size = size;
    vector->used = used;

    return vector;
}

// vector public api

USE_DECL
vector_t *vector_new(size_t size)
{
    return vector_init_inner(size, 0);
}

USE_DECL
vector_t *vector_of(size_t len)
{
    return vector_init_inner(len, len);
}

USE_DECL
vector_t *vector_init(void *value)
{
    vector_t *vector = vector_of(1);
    vector_set(vector, 0, value);
    return vector;
}

USE_DECL
vector_t *vector_clone(vector_t *vector)
{
    CTASSERT(vector != NULL);

    size_t len = vector_len(vector);
    vector_t *clone = vector_of(len);
    memcpy(clone->data, vector->data, len * sizeof(void *));
    return clone;
}

USE_DECL
void vector_delete(vector_t *vector)
{
    CTASSERT(vector != NULL);

    ctu_free(vector);
}

USE_DECL
void vector_push(vector_t **vector, void *value)
{
    CHECK_VECTOR(vector);

    vector_ensure(vector, VEC->used + 1);
    VEC->data[VEC->used++] = value;
}

USE_DECL
void vector_drop(vector_t *vector)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > 0);
    vector->used -= 1;
}

USE_DECL
void vector_set(vector_t *vector, size_t index, void *value)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > index);

    vector->data[index] = value;
}

USE_DECL
void *vector_get(const vector_t *vector, size_t index)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > index);

    return vector->data[index];
}

USE_DECL
void *vector_tail(const vector_t *vector)
{
    CTASSERT(vector != NULL);
    CTASSERT(vector_len(vector) > 0);

    return vector->data[vector->used - 1];
}

USE_DECL
size_t vector_len(const vector_t *vector)
{
    CTASSERT(vector != NULL);

    return vector->used;
}

USE_DECL
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

USE_DECL
vector_t *vector_merge(const vector_t *lhs, const vector_t *rhs)
{
    size_t lhsLength = vector_len(lhs);
    size_t rhsLength = vector_len(rhs);

    size_t len = lhsLength + rhsLength;

    vector_t *out = vector_new(len);
    out->used = len;

    memcpy(out->data, lhs->data, lhsLength * sizeof(void *));
    memcpy(out->data + lhsLength, rhs->data, rhsLength * sizeof(void *));

    return out;
}

USE_DECL
void vector_append(vector_t **vector, const vector_t *other)
{
    CHECK_VECTOR(vector);

    size_t len = vector_len(other);

    vector_ensure(vector, VEC->used + len);
    memcpy(VEC->data + VEC->used, other->data, len * sizeof(void *));
    VEC->used += len;
}

USE_DECL
vector_t *vector_join(vector_t *vectors)
{
    CTASSERT(vectors != NULL);

    // find the total length for less memory allocations
    size_t total_len = 0;
    size_t len = vector_len(vectors);

    for (size_t i = 0; i < len; i++)
    {
        total_len += vector_len(vector_get(vectors, i));
    }

    vector_t *result = vector_of(total_len);
    size_t offset = 0;

    for (size_t i = 0; i < len; i++)
    {
        vector_t *vector = vector_get(vectors, i);
        size_t innerLen = vector_len(vector);

        memcpy(result->data + offset, vector->data, innerLen * sizeof(void *));
        offset += innerLen;
    }

    return result;
}

typedef int(*sort_cmp_t)(const void *, const void *);

// unwrap the lhs and rhs pointers and pass them to the cmp function
// they are actually void** pointers
static int vector_cmp(void *cmp, const void *lhs, const void *rhs)
{
    sort_cmp_t fn = (sort_cmp_t)cmp;
    void *l = *(void **)lhs;
    void *r = *(void **)rhs;

    return fn(l, r);
}

USE_DECL
void vector_sort(vector_t *vector, int (*cmp)(const void *, const void *))
{
    CTASSERT(vector != NULL);
    CTASSERT(cmp != NULL);

    CTU_UNUSED(vector_cmp);

    //void *fn = (void *)cmp;
    //qsort_s(vector->data, vector_len(vector), sizeof(void *), vector_cmp, fn);
}

USE_DECL
void vector_reset(vector_t *vec)
{
    CTASSERT(vec != NULL);

    vec->used = 0;
}
