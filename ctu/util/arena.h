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

typedef void*(*arena_alloc_t)(void *arena, size_t bytes);
typedef void(*arena_realloc_t)(void *arena, void **ptr, size_t old, size_t bytes);
typedef void(*arena_release_t)(void *arena, void *ptr, size_t bytes);

typedef struct {
    arena_alloc_t alloc;
    arena_realloc_t realloc;
    arena_release_t release;
    const char *name;
    void *data;
} arena_t;

arena_t new_arena(const char *name, 
                  size_t initial,
                  arena_alloc_t alloc, 
                  arena_realloc_t realloc, 
                  arena_release_t release) NONULL;

#define NEW_ARENA(name, initial, alloc, realloc, release) \
    new_arena(name, initial, (arena_alloc_t)alloc, (arena_realloc_t)realloc, (arena_release_t)release)

void arena_free(arena_t *arena, void *ptr, size_t bytes) NONULL;
void *arena_malloc(arena_t *arena, size_t bytes) NOTNULL(1) ALLOC(arena_free);
void arena_realloc(arena_t *arena, void **ptr, size_t previous, size_t bytes) NOTNULL(1) ALLOC(arena_free);

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
arena_t new_blockmap(const char *name, size_t width, size_t blocks);

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
arena_t new_bitmap(const char *name, size_t width, size_t blocks);
