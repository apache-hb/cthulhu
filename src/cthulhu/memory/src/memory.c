// SPDX-License-Identifier: LGPL-3.0-only

#include "memory/memory.h"

#include "arena/arena.h"
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

/// gmp arena managment

static arena_t *gGmpArena = NULL;

static void *ctu_gmp_malloc(size_t size)
{
    return ARENA_MALLOC(size, "gmp", gGmpArena, gGmpArena);
}

static void *ctu_gmp_realloc(void *ptr, size_t old_size, size_t new_size)
{
    // mini-gmp gives us zero in some cases (-1 + 1) causes this
    size_t old = old_size != 0 ? old_size : CT_ALLOC_SIZE_UNKNOWN;
    return arena_realloc(ptr, new_size, old, gGmpArena);
}

static void ctu_gmp_free(void *ptr, size_t size)
{
    // mini-gmp doesnt handle free size and always gives us zero
    size_t old = size != 0 ? size : CT_ALLOC_SIZE_UNKNOWN;
    arena_free(ptr, old, gGmpArena);
}

USE_DECL
void init_gmp_arena(arena_t *arena)
{
    CTASSERT(arena != NULL);

    gGmpArena = arena;
    mp_set_memory_functions(ctu_gmp_malloc, ctu_gmp_realloc, ctu_gmp_free);
}
