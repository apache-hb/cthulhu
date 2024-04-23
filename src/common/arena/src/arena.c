// SPDX-License-Identifier: LGPL-3.0-only

#include "arena/arena.h"

#include "base/panic.h"
#include "base/util.h"

/// these return NULL on failure

STA_DECL
char *arena_opt_strdup(const char *str, arena_t *arena)
{
    CTASSERT(str != NULL);

    size_t len = ctu_strlen(str);
    char *out = ARENA_OPT_MALLOC(len + 1, "strdup", NULL, arena);
    if (out == NULL) return NULL;

    ctu_memcpy(out, str, len);
    out[len] = '\0';

    return out;
}

STA_DECL
char *arena_opt_strndup(const char *str, size_t len, arena_t *arena)
{
    CTASSERT(str != NULL);

    char *out = ARENA_OPT_MALLOC(len + 1, "strndup", NULL, arena);
    if (out == NULL) return NULL;

    ctu_memcpy(out, str, len);
    out[len] = '\0';

    return out;
}

STA_DECL
void *arena_opt_memdup(const void *ptr, size_t size, arena_t *arena)
{
    CTASSERT(ptr != NULL);

    void *out = ARENA_OPT_MALLOC(size, "memdup", NULL, arena);
    if (out == NULL) return NULL;

    ctu_memcpy(out, ptr, size);

    return out;
}

STA_DECL
void *arena_opt_malloc(size_t size, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERTF(arena->fn_malloc != NULL, "malloc is not implemented for %s", arena->name);

    return arena->fn_malloc(size, arena_data(arena));
}

STA_DECL
void *arena_opt_malloc_info(size_t size, const char *name, const void *parent, arena_t *arena)
{
    void *ptr = arena_opt_malloc(size, arena);
    if (ptr == NULL) return NULL;

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

STA_DECL
void *arena_opt_realloc(void *ptr, size_t new_size, size_t old_size, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERTF(arena->fn_realloc != NULL, "realloc is not implemented for %s", arena->name);

    return arena->fn_realloc(ptr, new_size, old_size, arena_data(arena));
}

STA_DECL
void arena_opt_free(void *ptr, size_t size, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERTF(arena->fn_free != NULL, "free is not implemented for %s", arena->name);

    arena->fn_free(ptr, size, arena_data(arena));
}

/// strong oom handling

STA_DECL
char *arena_strdup(const char *str, arena_t *arena)
{
    char *out = arena_opt_strdup(str, arena);
    CTASSERT(out != NULL);
    return out;
}

STA_DECL
char *arena_strndup(const char *str, size_t len, arena_t *arena)
{
    char *out = arena_opt_strndup(str, len, arena);
    CTASSERT(out != NULL);
    return out;
}

STA_DECL
void *arena_memdup(const void *ptr, size_t size, arena_t *arena)
{
    void *out = arena_opt_memdup(ptr, size, arena);
    CTASSERT(out != NULL);
    return out;
}

STA_DECL
void *arena_malloc(size_t size, arena_t *arena)
{
    CTASSERT(size > 0);

    void *ptr = arena_opt_malloc(size, arena);
    CTASSERT(ptr != NULL);
    return ptr;
}

STA_DECL
void *arena_malloc_info(size_t size, const char *name, const void *parent, arena_t *arena)
{
    CTASSERT(size > 0);

    void *ptr = arena_opt_malloc_info(size, name, parent, arena);
    CTASSERTF(ptr != NULL, "failed to allocate %zu bytes in %s", size, arena->name);
    return ptr;
}

STA_DECL
void *arena_realloc(void *ptr, size_t new_size, size_t old_size, arena_t *arena)
{
    CTASSERT(ptr != NULL);
    CTASSERT(new_size > 0);
    CTASSERT(old_size > 0);

    void *out = arena_opt_realloc(ptr, new_size, old_size, arena);
    CTASSERT(out != NULL);
    return out;
}

STA_DECL
void arena_free(void *ptr, size_t size, arena_t *arena)
{
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    arena_opt_free(ptr, size, arena);
}

STA_DECL
void arena_rename(const void *ptr, const char *name, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(name != NULL);

    if (arena->fn_rename != NULL)
    {
        arena->fn_rename(ptr, name, arena_data(arena));
    }
}

STA_DECL
void arena_reparent(const void *ptr, const void *parent, arena_t *arena)
{
    CTASSERT(arena != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(parent != NULL);

    if (arena->fn_reparent != NULL)
    {
        arena->fn_reparent(ptr, parent, arena_data(arena));
    }
}

STA_DECL
void *arena_data(arena_t *arena)
{
    CTASSERT(arena != NULL);

    return arena->user;
}
