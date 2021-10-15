#include "arena.h"

#include "ctu/util/macros.h"

#include <stdint.h>

arena_t new_arena(const char *name, arena_alloc_t alloc, arena_realloc_t realloc, arena_release_t release, void *data) {
    arena_t arena = {
        .alloc = alloc,
        .realloc = realloc,
        .release = release,
        .name = name,
        .data = data
    };

    return arena;
}

void *arena_malloc(arena_t *arena, size_t bytes) {
    CTASSERTF(bytes > 0, "allocating object with size of 0 in arena `%s`", arena->name);
    return arena->alloc(arena->data, bytes);
}

void arena_realloc(arena_t *arena, void **ptr, size_t previous, size_t bytes) {
    CTASSERTF(bytes > 0, "reallocating object with previous size of 0 in arena `%s`", arena->name);
    arena->realloc(arena->data, ptr, previous, bytes);
}

void arena_free(arena_t *arena, void *ptr, size_t bytes) {
    CTASSERTF(ptr != NULL, "freeing NULL object with size of %zu in arena `%s`", arena->name, bytes);
    arena->release(arena->data, ptr, bytes);
}
