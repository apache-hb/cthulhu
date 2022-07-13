#include "base/memory.h"
#include "base/macros.h"
#include "base/panic.h"

#include <stdlib.h>
#include <gmp.h>

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
    .arenaFree = default_global_free,
    .data = NULL
};

/// global allocator

void *ctu_malloc(size_t size)
{
    CTASSERT(size > 0);

    return arena_malloc(&globalAlloc, size, NULL);
}

void *ctu_realloc(void *ptr, size_t newSize)
{
    CTASSERT(ptr != NULL);
    CTASSERT(newSize > 0);
    
    return arena_realloc(&globalAlloc, ptr, newSize, ALLOC_SIZE_UNKNOWN);
}

void ctu_free(void *ptr)
{
    return arena_free(&globalAlloc, ptr, ALLOC_SIZE_UNKNOWN);
}

/// arena allocator

void *arena_malloc(alloc_t *alloc, size_t size, const char *name)
{   
    CTASSERT(alloc != NULL);

    return alloc->arenaMalloc(alloc, size, name);
}

void *arena_realloc(alloc_t *alloc, void *ptr, size_t newSize, size_t oldSize)
{
    CTASSERT(alloc != NULL);

    return alloc->arenaRealloc(alloc, ptr, newSize, oldSize);
}

void arena_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc != NULL);

    return alloc->arenaFree(alloc, ptr, size);
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

void init_gmp(alloc_t *alloc)
{    
    gmpAlloc = alloc != NULL ? alloc : &globalAlloc;
    mp_set_memory_functions(ctu_gmp_malloc, ctu_gmp_realloc, ctu_gmp_free);
}
