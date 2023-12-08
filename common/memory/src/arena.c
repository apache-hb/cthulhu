#include "memory/arena.h"

#include "base/panic.h"

/// arena allocator

USE_DECL
void *arena_malloc(alloc_t *alloc, size_t size, const char *name, const void *parent)
{
    CTASSERT(alloc != NULL);
    CTASSERT(size > 0);

    malloc_event_t event = {
        .alloc = alloc,
        .size = size,

        .name = name,
        .parent = parent
    };

    void *ptr = alloc->arena_malloc(event);
    CTASSERTF(ptr != NULL, "alloc(%s) failed", name);

    return ptr;
}

USE_DECL
void *arena_realloc(alloc_t *alloc, void *ptr, size_t new_size, size_t old_size)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(new_size > 0);
    CTASSERT(old_size > 0);

    realloc_event_t event = {
        .alloc = alloc,
        .ptr = ptr,
        .new_size = new_size,
        .old_size = old_size
    };

    void *outptr = alloc->arena_realloc(event);
    CTASSERTF(outptr != NULL, "realloc(%zu) failed", new_size);

    return outptr;
}

USE_DECL
void arena_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    free_event_t event = {
        .alloc = alloc,
        .ptr = ptr,
        .size = size
    };

    alloc->arena_free(event);
}
