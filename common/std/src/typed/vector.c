#include "std/typed/vector.h"

#include "core/macros.h"

#include "memory/memory.h"
#include "base/panic.h"

#include <string.h>

typedef struct typevec_t {
    size_t size;
    size_t used;

    size_t type_size;

    void *data;
} typevec_t;

static typevec_t *typevec_create(size_t type_size, size_t len)
{
    CTASSERT(type_size > 0);

    size_t size = MAX(len, 1);

    typevec_t *vec = ctu_malloc(sizeof(typevec_t));
    vec->size = size;
    vec->used = 0;
    vec->type_size = type_size;
    vec->data = ctu_malloc(type_size * (size + 1));
    return vec;
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
size_t typevec_len(typevec_t *vec)
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
void typevec_get(typevec_t *vec, size_t index, void *dst)
{
    CTASSERT(vec != NULL);
    CTASSERT(dst != NULL);

    void *src = typevec_offset(vec, index);
    memcpy(dst, src, vec->type_size);
}

USE_DECL
void typevec_tail(typevec_t *vec, void *dst)
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
void *typevec_offset(typevec_t *vec, size_t index)
{
    CTASSERTF(index < typevec_len(vec), "index %zu out of bounds %zu", index, typevec_len(vec));

    return ((char*)vec->data) + (index * vec->type_size);
}

USE_DECL
void *typevec_data(typevec_t *vec)
{
    CTASSERT(vec != NULL);

    return vec->data;
}
