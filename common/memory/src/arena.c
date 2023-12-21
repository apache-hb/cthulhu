#include "memory/arena.h"

#include "base/panic.h"

/// arena allocator

USE_DECL
void *arena_malloc(size_t size, const char *name, const void *parent, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(size > 0);

    void *ptr = arena->fn_malloc(size, arena->user);
    CTASSERTF(ptr != NULL, "arena `%s` of %zu bytes failed", name, size);

    if (name != NULL)
    {
        arena_rename(ptr, name, arena);
    }

    if (parent != NULL)
    {
        arena_reparent(ptr, parent, arena);
    }

    return ptr;
}

USE_DECL
void *arena_realloc(void *ptr, size_t new_size, size_t old_size, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(new_size > 0);
    CTASSERT(old_size > 0);

    void *outptr = arena->fn_realloc(ptr, new_size, old_size, arena->user);
    CTASSERTF(outptr != NULL, "realloc(%zu) failed", new_size);

    return outptr;
}

USE_DECL
void arena_free(void *ptr, size_t size, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    arena->fn_free(ptr, size, arena->user);
}

USE_DECL
void arena_rename(const void *ptr, const char *name, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(name != NULL);

    if (arena->fn_rename == NULL)
        return;

    arena->fn_rename(ptr, name, arena->user);
}

USE_DECL
void arena_reparent(const void *ptr, const void *parent, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(parent != NULL);

    if (arena->fn_reparent == NULL)
        return;

    arena->fn_reparent(ptr, parent, arena->user);
}
