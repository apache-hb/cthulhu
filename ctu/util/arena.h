#pragma once

#include <stddef.h>
#include "macros.h"

/** 
 * poison default memory functions
 * using these defeats the point of using arenas
 * and malloc has an impressively large startup overhead
 * on first call
 */
//POISON(malloc realloc free) /// libc memory managment
//POISON(ctu_malloc ctu_realloc ctu_free) /// our memory managment

typedef void*(*arena_malloc_t)(void *arena, size_t bytes);
typedef void(*arena_realloc_t)(void *arena, void **ptr, size_t old, size_t bytes);
typedef void(*arena_free_t)(void *arena, void *ptr, size_t bytes);
typedef void(*arena_reset_t)(void *arena);

#define ARENA_MALLOC(pfn) ((arena_malloc_t)pfn)
#define ARENA_REALLOC(pfn) ((arena_realloc_t)pfn)
#define ARENA_FREE(pfn) ((arena_free_t)pfn)
#define ARENA_RESET(pfn) ((arena_reset_t)pfn)

typedef struct {
    arena_malloc_t alloc;
    arena_realloc_t realloc;
    arena_free_t release;
    arena_reset_t reset;
    
    const char *name;
    size_t size;
    void *data;
} arena_t;

/**
 * create a new arena object
 * 
 * @param arena the arena object to fill in
 * @param initial the initial allocated size
 *        includes all allocation headers and other data
 * @param data the data to use for the arena
 *             leave NULL to use mmap
 * 
 * @return the new arena object
 */
void new_arena(WEAK arena_t *arena, size_t initial) NONULL;
void reset_arena(WEAK arena_t *arena) NONULL;
void delete_arena(OWNED arena_t *arena) NONULL;

void arena_free(WEAK arena_t *arena, OWNED void *ptr, size_t bytes) NONULL;
WEAK void *arena_malloc(WEAK arena_t *arena, size_t bytes) NOTNULL(1) ALLOC(arena_free);
void arena_realloc(WEAK arena_t *arena, WEAK void **ptr, size_t previous, size_t bytes) NOTNULL(1) ALLOC(arena_free);

/**
 * an arena optimized for variable sized allocations 
 * with frequent realloc calls
 * 
 * @param name the name of the arena
 * @param width the estimated width of the allocations maximum size
 * @param blocks the number of blocks to initially allocate
 * 
 * @return the arena
 */
OWNED arena_t new_blockmap(const char *name, size_t width, size_t blocks);

/**
 * an arena optimized for fixed sized allocations
 * which cannot realloc
 * 
 * @param name the name of the arena
 * @param width the width of each allocation
 * @param blocks the number of blocks to initially allocate
 * 
 * @return the arena
 */
OWNED arena_t new_bitmap(const char *name, size_t width, size_t blocks);

/**
 * a bump allocator optimized for variable sized allocations
 * with no reallocation or free calls
 * 
 * @param name the name of the arena
 * @param size the initial size of the arena
 * 
 * @return the arena
 */
OWNED arena_t new_bump(const char *name, size_t size);

OWNED arena_t fixed_arena(arena_t arena, WEAK void *data, size_t size);
