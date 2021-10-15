#include "arena.h"

#include "ctu/util/macros.h"
#include "arena/util.h"

#include <stdint.h>

void new_arena(arena_t *arena, size_t initial) {
    size_t aligned = ALIGN4K(initial);
    void *data = MMAP_ARENA(aligned);
    
    CTASSERTF(data != MAP_FAILED, "arena [%s] failed to map %zu bytes", aligned);

    arena->data = data;
    arena->size = aligned;
}

void reset_arena(arena_t *arena) {
    CTASSERTF(arena->reset != NULL, "arena [%s] cannot reset", arena->name);

    arena->reset(arena->data);
}

void delete_arena(arena_t *arena) {
    UNMAP_ARENA(arena->data, arena->size);
}

void *arena_malloc(arena_t *arena, size_t bytes) {
    CTASSERTF(bytes > 0, "arena [%s] allocating object with size of 0", arena->name);
    
    void *ptr = arena->alloc(arena->data, bytes);
    CTASSERTF(ptr != NULL, "arena [%s] exhausted", arena->name);
    return ptr;
}

void arena_realloc(arena_t *arena, void **ptr, size_t previous, size_t bytes) {
    CTASSERTF(bytes > 0, "reallocating object with previous size of 0 in arena `%s`", arena->name);
    CTASSERTF(arena->realloc != NULL, "arena [%s] does not support reallocation", arena->name);
    
    arena->realloc(arena->data, ptr, previous, bytes);
    CTASSERTF(*ptr != NULL, "arena [%s] exhausted", arena->name);
}

void arena_free(arena_t *arena, void *ptr, size_t bytes) {
    CTASSERTF(ptr != NULL, "arena [%s] freeing NULL object with size of %zu", arena->name, bytes);
    CTASSERTF(arena->release != NULL, "arena [%s] does not support freeing", arena->name);
    CTASSERTF(bytes > 0, "arena [%s] freeing object with size of 0", arena->name);

    arena->release(arena->data, ptr, bytes);
}
