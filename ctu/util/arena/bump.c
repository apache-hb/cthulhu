#include "ctu/util/arena.h"
#include "ctu/util/report.h"

typedef struct {
    const char *name;
    size_t size;
    char *ptr;
    char base[];
} bump_t;

static size_t bump_size(size_t size) {
    return sizeof(bump_t) + size;
}

static void *bump_malloc(bump_t *arena, size_t size) {
    if (arena->ptr + size > arena->base + arena->size) {
        return NULL;
    }

    void *ptr = arena->ptr;
    arena->ptr += size;
    return ptr;
}

static void bump_reset(bump_t *arena) {
    arena->ptr = arena->base;
}

arena_t new_bump(const char *name, size_t size) {
    arena_t result = {
        .alloc = ARENA_MALLOC(bump_malloc),
        .reset = ARENA_RESET(bump_reset),
        .name = name
    };

    new_arena(&result, bump_size(size));

    bump_t *bump = result.data;
    bump->name = name;
    bump->size = size;
    bump->ptr = bump->base;

    return result;
}
