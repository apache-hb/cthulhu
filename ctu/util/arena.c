#include "arena.h"

#include <sys/mman.h>

#if 0
arena_t new_arena(const char *name, size_t initial) {
    void *data = mmap(
        /* addr = */ NULL, 
        /* size = */ initial, 
        /* prot = */ PROT_READ | PROT_WRITE,
        /* flags = */ MAP_ANONYMOUS | MAP_UNINITIALZED,
        /* fd = */ -1,
        /* offset = */ 0
    );

    arena_t arena = { 
        .name = name, 
        .data = data, 
        .cursor = 0, 
        .size = initial 
    };

    return arena;
}

void delete_arena(arena_t arena) {
    munmap(arena.data, arena.size);
}

void *arena_alloc(arena_t *arena, size_t size) {

}

void arena_realloc(arena_t *arena, void **ptr, size_t old, size_t size) {

}

void arena_release(arena_t *arena, void *ptr, size_t size) {

}
#endif