#pragma once

#include <stddef.h>
#include "macros.h"

/** 
 * poison default memory functions
 * using these defeats the point of using arenas
 * and malloc has an impressively large startup overhead
 * on first call
 */
POISON(malloc realloc free) /// libc memory managment
POISON(ctu_malloc ctu_realloc ctu_free) /// our memory managment

typedef struct {
    const char *name;
    void *data;
    size_t cursor;
    size_t size;
} arena_t;

arena_t new_arena(const char *name, size_t initial);
void delete_arena(arena_t arena);

void *arena_alloc(arena_t *arena, size_t size);
void arena_realloc(arena_t *arena, void **ptr, size_t old, size_t size);
void arena_release(arena_t *arena, void *ptr, size_t size);
