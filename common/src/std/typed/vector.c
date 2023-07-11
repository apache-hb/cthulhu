#include "std/typed/vector.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

typedef struct typevec_t {
    size_t size;
    size_t used;

    size_t typeSize;

    void *data;
} typevec_t;

static typevec_t *typevec_create(size_t typeSize, size_t len)
{
    CTASSERT(typeSize > 0);

    typevec_t *vec = ctu_malloc(sizeof(typevec_t));
    vec->size = len;
    vec->used = 0;
    vec->typeSize = typeSize;
    vec->data = ctu_malloc(typeSize * (len + 1));
    return vec;
}

typevec_t *typevec_new(size_t size, size_t len)
{
    return typevec_create(size, len);
}

typevec_t *typevec_of(size_t size, size_t len)
{
    typevec_t *self = typevec_create(size, len);
    self->used = len;
    return self;
}

typevec_t *typevec_init(size_t size, const void *value)
{
    typevec_t *self = typevec_create(size, 1);
    typevec_push(self, value);
    return self;
}

size_t typevec_len(typevec_t *vec)
{
    CTASSERT(vec != NULL);

    return vec->used;
}

void typevec_set(typevec_t *vec, size_t index, const void *src)
{
    CTASSERT(vec != NULL);
    CTASSERT(src != NULL);

    void *dst = typevec_offset(vec, index);
    memcpy(dst, src, vec->typeSize);
}

void typevec_get(typevec_t *vec, size_t index, void *dst)
{
    CTASSERT(vec != NULL);
    CTASSERT(dst != NULL);

    void *src = typevec_offset(vec, index);
    memcpy(dst, src, vec->typeSize);
}

void typevec_tail(typevec_t *vec, void *dst)
{
    CTASSERT(vec != NULL);
    CTASSERT(vec->used > 0);

    typevec_get(vec, vec->used - 1, dst);
}

void typevec_push(typevec_t *vec, const void *src)
{
    CTASSERT(vec != NULL);

    if (vec->used == vec->size)
    {
        vec->size *= 2;
        vec->data = ctu_realloc(vec->data, vec->size * vec->typeSize);
    }

    void *dst = typevec_offset(vec, vec->used++);
    memcpy(dst, src, vec->typeSize);
}

void typevec_pop(typevec_t *vec, void *dst)
{
    CTASSERT(vec != NULL);
    CTASSERT(vec->used > 0);

    void *src = typevec_offset(vec, --vec->used);
    memcpy(dst, src, vec->typeSize);
}

void *typevec_offset(typevec_t *vec, size_t index)
{
    CTASSERT(vec != NULL);
    CTASSERT(index < vec->used);

    return ((char*)vec->data) + (index * vec->typeSize);
}
