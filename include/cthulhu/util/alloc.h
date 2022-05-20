#pragma once

#include <stddef.h>

typedef void *(*alloc_malloc_t)(void *self, size_t size);
typedef void *(*alloc_realloc_t)(void *self, void *ptr, size_t newSize, size_t oldSize);
typedef void (*alloc_free_t)(void *self, void *ptr, size_t size);

typedef struct
{
    alloc_malloc_t malloc;
    alloc_realloc_t realloc;
    alloc_free_t free;

    void *state;
} alloc_t;

void *alloc_new(alloc_t *alloc, size_t size, const char *name, const void *parent);
void *alloc_resize(alloc_t *alloc, void *ptr, size_t newSize, size_t oldSize, const char *name, const void *parent);
void alloc_delete(alloc_t *alloc, void *ptr, size_t size);

/**
 * @brief get the global allocator
 * 
 * @return the global allocator implementation
 */
alloc_t *alloc_global(void);

/**
 * @brief create a bump allocator
 * 
 * @param base the pointer to the base of the memory block
 * @param size the size of the memory block
 * @return the created allocator or NULL if not enough memory was provided
 */
alloc_t *alloc_bump(void *base, size_t size);

/**
 * @brief wraps an allocator and panics if any allocation fails
 * 
 * this allocator should only be used as a stopgap when porting 
 * code that expects no failures.
 *
 * @param source the allocator to wrap
 * @return the "safe" allocator
 */
alloc_t *alloc_rusty(alloc_t *source);
