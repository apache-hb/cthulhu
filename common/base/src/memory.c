#include "base/memory.h"
#include "core/macros.h"
#include "base/panic.h"

#include <gmp.h>
#include <stdlib.h>

/// default global allocator

static void *default_global_malloc(alloc_t *alloc, size_t size, const char *name)
{
    CTU_UNUSED(alloc);
    CTU_UNUSED(name);

    return malloc(size);
}

static void *default_global_realloc(alloc_t *alloc, void *ptr, size_t new_size, size_t old_size)
{
    CTU_UNUSED(alloc);
    CTU_UNUSED(old_size);

    return realloc(ptr, new_size);
}

static void default_global_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTU_UNUSED(alloc);
    CTU_UNUSED(size);

    free(ptr);
}

alloc_t gDefaultAlloc = {
    .name = "default global allocator",
    .arena_malloc = default_global_malloc,
    .arena_realloc = default_global_realloc,
    .arena_free = default_global_free
};

/// global allocator

USE_DECL
void *ctu_malloc(size_t size)
{
    CTASSERT(size > 0);

    return arena_malloc(&gDefaultAlloc, size, "");
}

USE_DECL
void *ctu_realloc(void *ptr, size_t new_size)
{
    CTASSERT(ptr != NULL);
    CTASSERT(new_size > 0);

    return arena_realloc(&gDefaultAlloc, ptr, new_size, ALLOC_SIZE_UNKNOWN);
}

USE_DECL
void ctu_free(void *ptr)
{
    arena_free(&gDefaultAlloc, ptr, ALLOC_SIZE_UNKNOWN);
}

/// arena allocator

USE_DECL
void *arena_malloc(alloc_t *alloc, size_t size, const char *name)
{
    CTASSERT(alloc != NULL);
    CTASSERT(size > 0);

    void *ptr = alloc->arena_malloc(alloc, size, name);
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

    void *outptr = alloc->arena_realloc(alloc, ptr, new_size, old_size);
    CTASSERTF(outptr != NULL, "realloc(%zu) failed", new_size);

    return outptr;
}

USE_DECL
void arena_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc != NULL);
    CTASSERT(ptr != NULL);
    CTASSERT(size > 0);

    alloc->arena_free(alloc, ptr, size);
}

/// gmp arena managment

static alloc_t *gGmpAlloc = NULL;

static void *ctu_gmp_malloc(size_t size)
{
    return arena_malloc(gGmpAlloc, size, "gmp-alloc");
}

static void *ctu_gmp_realloc(void *ptr, size_t old_size, size_t new_size)
{
    return arena_realloc(gGmpAlloc, ptr, new_size, old_size);
}

static void ctu_gmp_free(void *ptr, size_t size)
{
    // mini-gmp doesnt handle free size and always gives us zero
    arena_free(gGmpAlloc, ptr, size != 0 ? size : ALLOC_SIZE_UNKNOWN);
}

USE_DECL
void init_gmp(alloc_t *alloc)
{
    CTASSERT(alloc != NULL);

    gGmpAlloc = alloc;
    mp_set_memory_functions(ctu_gmp_malloc, ctu_gmp_realloc, ctu_gmp_free);
}
