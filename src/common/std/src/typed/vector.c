#include "std/typed/vector.h"

#include "base/util.h"
#include "core/macros.h"

#include "arena/arena.h"
#include "base/panic.h"

#include <stdlib.h>

/// @brief A vector with a fixed type size.
typedef struct typevec_t
{
    arena_t *arena;

    /// @brief The number of elements allocated.
    size_t size;

    /// @brief The number of elements used.
    size_t used;

    /// @brief The size of each element.
    size_t type_size;

    /// @brief The data of the vector.
    FIELD_SIZE(size) void *data;
} typevec_t;

// seperate from typevec_offset because we want to be able to get offsets
// outside of the vector
static void *get_element_offset(const typevec_t *vec, size_t index)
{
    CTASSERT(vec != NULL);

    return ((char*)vec->data) + (index * vec->type_size);
}

static void copy_elements(const typevec_t *vec, void *dst, const void *src, size_t count)
{
    CTASSERT(vec != NULL);

    ctu_memcpy(dst, src, vec->type_size * count);
}

static void typevec_ensure(typevec_t *vec, size_t extra)
{
    CTASSERT(vec != NULL);

    if (vec->used + extra > vec->size)
    {
        size_t new_size = MAX(vec->size * 2, vec->used + extra);

        vec->data = arena_realloc(vec->data, new_size * vec->type_size, vec->size * vec->type_size, vec->arena);

        vec->size = new_size;
    }
}

static typevec_t *typevec_create(size_t type_size, size_t len, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(type_size > 0);

    size_t size = MAX(len, 1);

    typevec_t *vec = ARENA_MALLOC(sizeof(typevec_t), "typevec", NULL, arena);
    vec->arena = arena;
    vec->size = size;
    vec->used = 0;
    vec->type_size = type_size;
    vec->data = ARENA_MALLOC(type_size * (size + 1), "typevec_data", vec, arena);
    ARENA_IDENTIFY(vec->data, "data", vec, arena);

    return vec;
}

USE_DECL
typevec_t *typevec_new(size_t type_size, size_t len, arena_t *arena)
{
    return typevec_create(type_size, len, arena);
}

USE_DECL
typevec_t *typevec_of(size_t type_size, size_t len, arena_t *arena)
{
    typevec_t *self = typevec_create(type_size, len, arena);
    self->used = len;
    return self;
}

USE_DECL
typevec_t *typevec_of_array(size_t type_size, const void *src, size_t count, arena_t *arena)
{
    CTASSERT(src != NULL);

    typevec_t *self = typevec_create(type_size, count, arena);
    self->used = count;

    copy_elements(self, self->data, src, count);

    return self;
}

USE_DECL
typevec_t *typevec_slice(const typevec_t *vec, size_t start, size_t end)
{
    CTASSERT(vec != NULL);
    CTASSERTF(start <= end, "start %zu > end %zu", start, end);
    CTASSERTF(end <= typevec_len(vec), "end %zu out of bounds %zu", end, typevec_len(vec));

    size_t len = end - start;
    typevec_t *self = typevec_create(vec->type_size, len, vec->arena);
    self->used = len;

    copy_elements(self, self->data, typevec_offset(vec, start), len);

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
    copy_elements(vec, dst, src, 1);
}

USE_DECL
void typevec_get(const typevec_t *vec, size_t index, void *dst)
{
    CTASSERT(vec != NULL);
    CTASSERT(dst != NULL);

    void *src = typevec_offset(vec, index);
    copy_elements(vec, dst, src, 1);
}

USE_DECL
void typevec_tail(const typevec_t *vec, void *dst)
{
    CTASSERT(typevec_len(vec) > 0);

    typevec_get(vec, vec->used - 1, dst);
}

USE_DECL
void *typevec_push(typevec_t *vec, const void *src)
{
    CTASSERT(vec != NULL);

    typevec_ensure(vec, 1);

    void *dst = get_element_offset(vec, vec->used++);
    copy_elements(vec, dst, src, 1);

    return dst;
}

USE_DECL
void typevec_append(typevec_t *vec, const void *src, size_t len)
{
    CTASSERT(vec != NULL);
    CTASSERT(src != NULL);

    typevec_ensure(vec, len);

    void *dst = get_element_offset(vec, vec->used);
    ctu_memcpy(dst, src, vec->type_size * len);
    vec->used += len;
}

USE_DECL
void typevec_pop(typevec_t *vec, void *dst)
{
    CTASSERT(typevec_len(vec) > 0);

    void *src = typevec_offset(vec, --vec->used);
    copy_elements(vec, dst, src, 1);
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

USE_DECL
void typevec_reset(typevec_t *vec)
{
    CTASSERT(vec != NULL);

    vec->used = 0;
}
