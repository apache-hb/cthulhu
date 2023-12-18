#include "std/typed/vector.h"

#include "core/macros.h"

#include "memory/memory.h"
#include "base/panic.h"

#include <stdlib.h>
#include <string.h>

/// @brief A vector with a fixed type size.
typedef struct typevec_t
{
    /// @brief The number of elements allocated.
    size_t size;

    /// @brief The number of elements used.
    size_t used;

    /// @brief The size of each element.
    size_t type_size;

    /// @brief The data of the vector.
    void *data;
} typevec_t;

static typevec_t *typevec_create(size_t type_size, size_t len)
{
    CTASSERT(type_size > 0);

    size_t size = MAX(len, 1);

    typevec_t *vec = MEM_ALLOC(sizeof(typevec_t), "typevec", NULL);
    vec->size = size;
    vec->used = 0;
    vec->type_size = type_size;
    vec->data = MEM_ALLOC(type_size * (size + 1), "typevec_data", vec);
    return vec;
}

void typevec_delete(typevec_t *vec)
{
    CTASSERT(vec != NULL);

    ctu_free(vec->data);
    ctu_free(vec);
}

USE_DECL
typevec_t *typevec_new(size_t size, size_t len)
{
    return typevec_create(size, len);
}

USE_DECL
typevec_t *typevec_of(size_t size, size_t len)
{
    typevec_t *self = typevec_create(size, len);
    self->used = len;
    return self;
}

USE_DECL
typevec_t *typevec_init(size_t size, const void *value)
{
    typevec_t *self = typevec_create(size, 1);
    typevec_push(self, value);
    return self;
}

USE_DECL
size_t typevec_len(const typevec_t *vec)
{
    CTASSERT(vec != NULL);

    return vec->used;
}

USE_DECL
void typevec_set(typevec_t *vec, size_t index, const void *src)
{
    CTASSERT(vec != NULL);
    CTASSERT(src != NULL);

    void *dst = typevec_offset(vec, index);
    memcpy(dst, src, vec->type_size);
}

USE_DECL
void typevec_get(const typevec_t *vec, size_t index, void *dst)
{
    CTASSERT(vec != NULL);
    CTASSERT(dst != NULL);

    void *src = typevec_offset(vec, index);
    memcpy(dst, src, vec->type_size);
}

USE_DECL
void typevec_tail(const typevec_t *vec, void *dst)
{
    CTASSERT(typevec_len(vec) > 0);

    typevec_get(vec, vec->used - 1, dst);
}

USE_DECL
void typevec_push(typevec_t *vec, const void *src)
{
    CTASSERT(vec != NULL);

    if (vec->used == vec->size)
    {
        vec->size *= 2;
        vec->data = ctu_realloc(vec->data, vec->size * vec->type_size);
    }

    void *dst = typevec_offset(vec, vec->used++);
    memcpy(dst, src, vec->type_size);
}

USE_DECL
void typevec_pop(typevec_t *vec, void *dst)
{
    CTASSERT(typevec_len(vec) > 0);

    void *src = typevec_offset(vec, --vec->used);
    memcpy(dst, src, vec->type_size);
}

USE_DECL
void *typevec_offset(const typevec_t *vec, size_t index)
{
    CTASSERTF(index < typevec_len(vec), "index %zu out of bounds %zu", index, typevec_len(vec));

    return ((char*)vec->data) + (index * vec->type_size);
}

USE_DECL
void *typevec_data(const typevec_t *vec)
{
    CTASSERT(vec != NULL);

    return vec->data;
}

void typevec_sort(IN_NOTNULL typevec_t *vec, int (*cmp)(const void *, const void *))
{
    CTASSERT(vec != NULL);
    CTASSERT(cmp != NULL);

    // TODO: should we rely on qsort existing?

    qsort(vec->data, vec->used, vec->type_size, cmp);
}
