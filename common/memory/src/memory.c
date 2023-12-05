#include "memory/memory.h"

#include "core/macros.h"
#include "base/panic.h"

#include <gmp.h>

#include <string.h>
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

/// global string allocation

USE_DECL
void *ctu_memdup(const void *ptr, size_t size)
{
    CTASSERT(ptr != NULL);

    void *out = ctu_malloc(size);
    memcpy(out, ptr, size);
    return out;
}

USE_DECL
char *ctu_strdup(const char *str)
{
    CTASSERT(str != NULL);

    size_t len = strlen(str) + 1;
    char *out = ctu_malloc(len);
    memcpy(out, str, len);
    return out;
}

USE_DECL
char *ctu_strndup(const char *str, size_t len)
{
    CTASSERT(str != NULL);

    char *out = ctu_malloc(len + 1);
    memcpy(out, str, len);
    out[len] = '\0';
    return out;
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