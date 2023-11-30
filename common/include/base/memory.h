#pragma once

#include "base/analyze.h"

#include <stddef.h>
#include <stdint.h>

BEGIN_API

#define ALLOC_SIZE_UNKNOWN SIZE_MAX

typedef struct alloc_t alloc_t;

typedef void *(*malloc_t)(alloc_t *self, size_t size, const char *name);
typedef void *(*realloc_t)(alloc_t *self, void *ptr, size_t new_size, size_t old_size);
typedef void (*free_t)(alloc_t *self, void *ptr, size_t size);

/**
 * @brief an allocator
 */
typedef struct alloc_t
{
    const char *name;
    malloc_t arena_malloc;
    realloc_t arena_realloc;
    free_t arena_free;
} alloc_t;

extern alloc_t gDefaultAlloc;

// C allocators

void ctu_free(IN_NOTNULL void *ptr);

NODISCARD ALLOC(ctu_free)
void *ctu_malloc(size_t size);

NODISCARD ALLOC(ctu_free)
void *ctu_realloc(IN_NOTNULL void *ptr, size_t new_size);

// gmp

void init_gmp(IN_NOTNULL alloc_t *alloc);

// arena allocators

void arena_free(IN_NOTNULL alloc_t *alloc, IN_NOTNULL void *ptr, size_t size);

NODISCARD ALLOC(arena_free, 2)
void *arena_malloc(IN_NOTNULL alloc_t *alloc, size_t size, const char *name);

NODISCARD
void *arena_realloc(IN_NOTNULL alloc_t *alloc, IN_NOTNULL void *ptr, size_t new_size, size_t old_size);

END_API
