#include "arena/arena.h"

#include "base/panic.h"

#include <string.h>

/// arena allocator

USE_DECL
char *arena_strdup(const char *str, arena_t *arena)
{
    CTASSERT(str != NULL);

    size_t len = strlen(str);
    char *out = arena_malloc(len + 1, "strdup", arena, arena);
    memcpy(out, str, len);
    out[len] = '\0';
    return out;
}

USE_DECL
char *arena_strndup(const char *str, size_t len, arena_t *arena)
{
    CTASSERT(str != NULL);

    char *out = arena_malloc(len + 1, "strndup", arena, arena);
    memcpy(out, str, len);
    out[len] = '\0';
    return out;
}

USE_DECL
void *arena_memdup(const void *ptr, size_t size, arena_t *arena)
{
    CTASSERT(ptr != NULL);

    void *out = arena_malloc(size, "memdup", arena, arena);
    memcpy(out, ptr, size);
    return out;
}

USE_DECL
void *arena_malloc(size_t size, const char *name, const void *parent, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(size > 0);

    void *ptr = arena->fn_malloc(size, arena_data(arena));
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

    void *outptr = arena->fn_realloc(ptr, new_size, old_size, arena_data(arena));
    CTASSERTF(outptr != NULL, "realloc(%zu) failed", new_size);

    return outptr;
}

USE_DECL
void arena_free(void *ptr, size_t size, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    arena->fn_free(ptr, size, arena_data(arena));
}

USE_DECL
void arena_rename(const void *ptr, const char *name, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(name != NULL);

    if (arena->fn_rename == NULL) return;

    arena->fn_rename(ptr, name, arena_data(arena));
}

USE_DECL
void arena_reparent(const void *ptr, const void *parent, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(parent != NULL);

    if (arena->fn_reparent == NULL) return;

    arena->fn_reparent(ptr, parent, arena_data(arena));
}

USE_DECL
void *arena_data(arena_t *arena)
{
    CTASSERT(arena != NULL);

    return arena->user;
}
