#pragma once

#include <stddef.h>

typedef struct {
    const char *name;
    void *data;
    size_t size;
} arena_t;

arena_t new_arena(const char *name, size_t initial);
void delete_arena(arena_t arena);

void *arena_alloc(arena_t *arena, size_t size);
void arena_realloc(arena_t *arena, void **ptr, size_t old, size_t size);
void arena_release(arena_t *arena, void *ptr, size_t size);
