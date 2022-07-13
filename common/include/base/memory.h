#pragma once

#include "base/analyze.h"

#include <stddef.h>
#include <stdint.h>

typedef struct alloc_t alloc_t;

typedef void *(*malloc_t)(alloc_t *self, size_t size, const char *name);
typedef void *(*realloc_t)(alloc_t *self, void *ptr, size_t newSize, size_t oldSize);
typedef void (*free_t)(alloc_t *self, void *ptr, size_t size);

/**
 * @brief an allocator
 */
typedef struct alloc_t
{
    const char *name;
    malloc_t arenaMalloc;
    realloc_t arenaRealloc;
    free_t arenaFree;
    void *data;
} alloc_t;

extern alloc_t globalAlloc;

#define ALLOC_SIZE_UNKNOWN SIZE_MAX

NODISCARD void *ctu_malloc(size_t size);
NODISCARD void *ctu_realloc(void *ptr, size_t newSize);
void ctu_free(void *ptr);

void init_gmp(alloc_t *alloc);

NODISCARD void *arena_malloc(alloc_t *alloc, size_t size, const char *name);
NODISCARD void *arena_realloc(alloc_t *alloc, void *ptr, size_t newSize, size_t oldSize);

void arena_free(alloc_t *alloc, void *ptr, size_t size);
