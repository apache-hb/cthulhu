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
    void *data;
} arena_t;

arena_t new_arena(arena_alloc_t alloc, arena_realloc_t realloc, arena_release_t release, void *data);
#define NEW_ARENA(alloc, realloc, release, data) \
    new_arena((arena_alloc_t)alloc, (arena_realloc_t)realloc, (arena_release_t)release, (void*)data)

void *arena_malloc(arena_t *arena, size_t bytes);
void arena_realloc(arena_t *arena, void **ptr, size_t previous, size_t bytes);
void arena_free(arena_t *arena, void *ptr, size_t bytes);

arena_t new_blockmap(size_t width, size_t blocks);
