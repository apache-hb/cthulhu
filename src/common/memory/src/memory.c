#include "memory/memory.h"

#include "base/panic.h"

#include <gmp.h>

#include <string.h>
#include <stdlib.h>

///
/// global allocator
///

static arena_t *gGlobalArena = NULL;

arena_t *get_global_arena(void)
{
    return gGlobalArena;
}

void init_global_arena(arena_t *arena)
{
    CTASSERT(arena != NULL);

    gGlobalArena = arena;
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
