#include "base/memory.h"
#include "base/macros.h"
#include "base/panic.h"

#include <gmp.h>
#include <stdlib.h>

/// default global allocator

static void *default_global_malloc(alloc_t *alloc, size_t size, const char *name)
{
    UNUSED(alloc);
    UNUSED(name);

    return malloc(size);
}

static void *default_global_realloc(alloc_t *alloc, void *ptr, size_t newSize, size_t oldSize)
{
    UNUSED(alloc);
    UNUSED(oldSize);

    return realloc(ptr, newSize);
}

static void default_global_free(alloc_t *alloc, void *ptr, size_t size)
{
    UNUSED(alloc);
    UNUSED(size);

    free(ptr);
}

alloc_t globalAlloc = {
    .name = "default global allocator",
    .arenaMalloc = default_global_malloc,
    .arenaRealloc = default_global_realloc,
    .arenaFree = default_global_free
};

/// global allocator

USE_DECL
void *ctu_malloc(size_t size)
{
    CTASSERT(size > 0);

    return arena_malloc(&globalAlloc, size, "");
}

USE_DECL
void *ctu_realloc(void *ptr, size_t newSize)
{
    CTASSERT(ptr != NULL);
    CTASSERT(newSize > 0);

    return arena_realloc(&globalAlloc, ptr, newSize, ALLOC_SIZE_UNKNOWN);
}

USE_DECL
void ctu_free(void *ptr)
{
    arena_free(&globalAlloc, ptr, ALLOC_SIZE_UNKNOWN);
}

/// arena allocator

USE_DECL
void *arena_malloc(alloc_t *alloc, size_t size, const char *name)
{
    CTASSERT(alloc != NULL);

    return alloc->arenaMalloc(alloc, size, name);
}

USE_DECL
void *arena_realloc(alloc_t *alloc, void *ptr, size_t newSize, size_t oldSize)
{
    CTASSERT(alloc != NULL);

    return alloc->arenaRealloc(alloc, ptr, newSize, oldSize);
}

USE_DECL
void arena_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc != NULL);

    alloc->arenaFree(alloc, ptr, size);
}

/// gmp arena managment

static alloc_t *gmpAlloc = NULL;

static void *ctu_gmp_malloc(size_t size)
{
    return arena_malloc(gmpAlloc, size, "gmp-alloc");
}

static void *ctu_gmp_realloc(void *ptr, size_t oldSize, size_t newSize)
{
    return arena_realloc(gmpAlloc, ptr, newSize, oldSize);
}

static void ctu_gmp_free(void *ptr, size_t size)
{
    arena_free(gmpAlloc, ptr, size);
}

USE_DECL
void init_gmp(alloc_t *alloc)
{
    CTASSERT(alloc != NULL);

    gmpAlloc = alloc;
    mp_set_memory_functions(ctu_gmp_malloc, ctu_gmp_realloc, ctu_gmp_free);
}
