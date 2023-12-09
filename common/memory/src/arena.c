#include "memory/arena.h"

#include "base/panic.h"

typedef struct mem_t
{
    arena_t *alloc;
} mem_t;

static mem_t event_new(arena_t *alloc)
{
    CTASSERT(alloc != NULL);

    mem_t event = {
        .alloc = alloc
    };

    return event;
}

arena_t *mem_arena(const mem_t *event)
{
    CTASSERT(event != NULL);

    return event->alloc;
}

/// arena allocator

USE_DECL
void *arena_malloc(arena_t *alloc, size_t size, const char *name, const void *parent)
{
    CTASSERT(alloc != NULL);
    CTASSERT(size > 0);

    mem_t mem = event_new(alloc);

    void *ptr = alloc->fn_malloc(&mem, size);
    CTASSERTF(ptr != NULL, "alloc(%s) failed", name);

    if (name != NULL)
    {
        arena_rename(alloc, ptr, name);
    }

    if (parent != NULL)
    {
        arena_reparent(alloc, ptr, parent);
    }

    return ptr;
}

USE_DECL
void *arena_realloc(arena_t *alloc, void *ptr, size_t new_size, size_t old_size)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(new_size > 0);
    CTASSERT(old_size > 0);

    mem_t mem = event_new(alloc);

    void *outptr = alloc->fn_realloc(&mem, ptr, new_size, old_size);
    CTASSERTF(outptr != NULL, "realloc(%zu) failed", new_size);

    return outptr;
}

USE_DECL
void arena_free(arena_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    mem_t mem = event_new(alloc);

    alloc->fn_free(&mem, ptr, size);
}

USE_DECL
void arena_rename(arena_t *alloc, const void *ptr, const char *name)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(name != NULL);

    if (alloc->fn_rename == NULL)
        return;

    mem_t mem = event_new(alloc);

    alloc->fn_rename(&mem, ptr, name);
}

USE_DECL
void arena_reparent(arena_t *alloc, const void *ptr, const void *parent)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(parent != NULL);

    if (alloc->fn_reparent == NULL)
        return;

    mem_t mem = event_new(alloc);

    alloc->fn_reparent(&mem, ptr, parent);
}
