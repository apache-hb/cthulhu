#include "arena/bitmap.h"

#include "arena/arena.h"
#include "base/panic.h"

#include <string.h>

static void bitmap_arena_free(void *ptr, size_t size, void *user)
{
    bitmap_free(user, ptr, size);
}

static void *bitmap_arena_alloc(size_t size, void *user)
{
    return bitmap_alloc(user, size);
}

void bitmap_arena_init(arena_t *self, bitmap_t *bitmap)
{
    CTASSERT(self != NULL);
    CTASSERT(bitmap != NULL);
    self->user = bitmap;
    self->fn_malloc = bitmap_arena_alloc;
    self->fn_free = bitmap_arena_free;
}

bitmap_t *bitmap_new(size_t count, size_t stride, arena_t *arena)
{
    CTASSERT(count > 0);
    CTASSERT(stride > 0);
    CTASSERT(arena != NULL);

    bitmap_t *self = ARENA_MALLOC(sizeof(bitmap_t), "bitmap arena", arena, arena);
    bitmap_init(self, count, stride, arena);
    return self;
}

void bitmap_init(bitmap_t *bitmap, size_t count, size_t stride, arena_t *arena)
{
    CTASSERT(bitmap != NULL);
    CTASSERT(count > 0);
    CTASSERT(stride > 0);
    CTASSERT(arena != NULL);

    void *data = ARENA_MALLOC(count * stride, "bitmap arena data", bitmap, arena);

    size_t length = (count / CHAR_BIT);
    void *bitset = ARENA_MALLOC(length, "backing bitset", bitmap, arena);

    memset(bitset, 0, length);

    bitmap_t result = {
        .data = data,
#if CTU_DEBUG
        .stride = stride,
#endif
        .bitset = bitset_of(bitset, length),
        .parent = arena,
    };

    *bitmap = result;
}

static size_t bitmap_alloc_bit(bitmap_t *arena)
{
    size_t slot = bitset_set_first(arena->bitset, arena->index);
    if (slot == SIZE_MAX)
        slot = bitset_set_first(arena->bitset, 0);

    return slot;
}

static void *bitmap_mark_bit(bitmap_t *self, size_t index, size_t stride)
{
    self->index = index;
    return self->data + (index * stride);
}

static void *bitmap_alloc_next(bitmap_t *self, size_t size)
{
    CTASSERT(self != NULL);

    bitmap_t *next = self->next;
    if (next == NULL)
    {
        next = bitmap_new(bitset_len(self->bitset), size, self->parent);
        self->next = next;
    }

    return bitmap_alloc(next, size);
}

void *bitmap_alloc(bitmap_t *bitmap, size_t size)
{
#if CTU_DEBUG
    CTASSERTF(size == bitmap->stride, "bitmap arena: size mismatch (expected %zu, got %zu)", bitmap->stride, size);
#endif

    // find a free slot
    size_t slot = bitmap_alloc_bit(bitmap);

    // if we still didnt find anything, go to the next arena
    if (slot == SIZE_MAX)
        return bitmap_alloc_next(bitmap, size);

    return bitmap_mark_bit(bitmap, slot, size);
}

static bool bitmap_contains(char *ptr, bitmap_t *arena, size_t stride)
{
    return ptr >= arena->data && ptr < (arena->data + bitset_len(arena->bitset) * stride);
}

static bool bitmap_free_next(char *ptr, size_t size, bitmap_t *arena)
{
    if (arena == NULL)
        return false;

    if (!bitmap_contains(ptr, arena, size))
        return bitmap_free_next(ptr, size, arena->next);

    size_t slot = (ptr - arena->data) / size;
    bitset_clear(arena->bitset, slot);
    return true;
}

void bitmap_free(bitmap_t *bitmap, void *ptr, size_t size)
{
#if CTU_DEBUG
    CTASSERTF(size == bitmap->stride, "bitmap arena: size mismatch (expected %zu, got %zu)", bitmap->stride, size);
    CTASSERTF(((uintptr_t)ptr) % bitmap->stride == 0, "bitmap arena: misaligned pointer %p (not aligned to %zu)", ptr, bitmap->stride);
#endif

    if (bitmap_free_next(ptr, size, bitmap))
        return;

    CT_NEVER("bitmap arena: invalid pointer %p", ptr);
}
