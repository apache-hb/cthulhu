#include "memory/memory.h"

#include "base/panic.h"
#include "core/macros.h"

#include <gmp.h>

#include <string.h>
#include <stdlib.h>

/// default global allocator

static void *default_malloc(size_t size, void *user)
{
    CTU_UNUSED(user);

    return malloc(size);
}

static void *default_realloc(void *ptr, size_t new_size, size_t old_size, void *user)
{
    CTU_UNUSED(user);
    CTU_UNUSED(old_size);

    return realloc(ptr, new_size);
}

static void default_free(void *ptr, size_t size, void *user)
{
    CTU_UNUSED(user);
    CTU_UNUSED(size);

    free(ptr);
}

static arena_t gDefaultAlloc = {
    .name = "default global allocator",
    .fn_malloc = default_malloc,
    .fn_realloc = default_realloc,
    .fn_free = default_free
};

arena_t *ctu_default_alloc(void)
{
    return &gDefaultAlloc;
}

///
/// global allocator
///

static arena_t *gGlobalArena = NULL;

arena_t *get_global_arena(void)
{
    CTASSERT(gGlobalArena != NULL);

    return gGlobalArena;
}

void init_global_arena(arena_t *arena)
{
    CTASSERT(arena != NULL);

    gGlobalArena = arena;
}

void ctu_malloc(size_t size)
{
    arena_malloc(size, "global", gGlobalArena, get_global_arena());
}

void *ctu_realloc(void *ptr, size_t new_size, size_t old_size)
{
    return arena_realloc(ptr, new_size, old_size, get_global_arena());
}

void ctu_free(void *ptr, size_t size)
{
    arena_free(ptr, size, get_global_arena());
}


USE_DECL
void *ctu_memdup(const void *ptr, size_t size)
{
    return arena_memdup(ptr, size, get_global_arena());
}

USE_DECL
char *ctu_strdup(const char *str)
{
    return arena_strdup(str, get_global_arena());
}

USE_DECL
char *ctu_strndup(const char *str, size_t len)
{
    return arena_strndup(str, len, get_global_arena());
}

/// gmp arena managment

static arena_t *gGmpArena = NULL;

static void *ctu_gmp_malloc(size_t size)
{
    return arena_malloc(size, "gmp", gGmpArena, gGmpArena);
}

static void *ctu_gmp_realloc(void *ptr, size_t old_size, size_t new_size)
{
    return arena_realloc(ptr, new_size, old_size, gGmpArena);
}

static void ctu_gmp_free(void *ptr, size_t size)
{
    // mini-gmp doesnt handle free size and always gives us zero
    arena_free(ptr, size != 0 ? size : ALLOC_SIZE_UNKNOWN, gGmpArena);
}

USE_DECL
void init_gmp_arena(arena_t *arena)
{
    CTASSERT(arena != NULL);

    gGmpArena = arena;
    mp_set_memory_functions(ctu_gmp_malloc, ctu_gmp_realloc, ctu_gmp_free);
}
