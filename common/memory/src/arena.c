#include "memory/arena.h"

#include "base/panic.h"

typedef struct mem_t
{
    alloc_t *alloc;
} mem_t;

static mem_t event_new(alloc_t *alloc)
{
    CTASSERT(alloc != NULL);

    mem_t event = {
        .alloc = alloc
    };

    return event;
}

alloc_t *mem_arena(const mem_t *event)
{
    CTASSERT(event != NULL);

    return event->alloc;
}

/// arena allocator

USE_DECL
void *arena_malloc(alloc_t *alloc, size_t size, const char *name, const void *parent)
{
    CTASSERT(alloc != NULL);
    CTASSERT(size > 0);

    mem_t mem = event_new(alloc);

    malloc_event_t event = {
        .size = size,

        .name = name,
        .parent = parent
    };

    void *ptr = alloc->arena_malloc(&mem, event);
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

    mem_t mem = event_new(alloc);

    realloc_event_t event = {
        .ptr = ptr,
        .new_size = new_size,
        .old_size = old_size
    };

    void *outptr = alloc->arena_realloc(&mem, event);
    CTASSERTF(outptr != NULL, "realloc(%zu) failed", new_size);

    return outptr;
}

USE_DECL
void arena_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    mem_t mem = event_new(alloc);

    free_event_t event = {
        .ptr = ptr,
        .size = size
    };

    alloc->arena_free(&mem, event);
}
