#include "memory/arena.h"

#include "base/panic.h"

/// arena allocator

USE_DECL
void *arena_malloc(alloc_t *alloc, size_t size, const char *name, const void *parent)
{
    CTASSERT(alloc != NULL);
    CTASSERT(size > 0);

    void *ptr = alloc->arena_malloc(alloc, size, name, parent);
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

    void *outptr = alloc->arena_realloc(alloc, ptr, new_size, old_size);
    CTASSERTF(outptr != NULL, "realloc(%zu) failed", new_size);

    return outptr;
}

USE_DECL
void arena_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    alloc->arena_free(alloc, ptr, size);
}
