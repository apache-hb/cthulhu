#include "ctu/util/arena.h"

#include "util.h"

#include <stdint.h>

typedef struct {
    const char *name;
    size_t blocks; /* total number of allocated blocks */
    size_t width; /* the width of each block */
    char data[]; /* the counters and then block data */
} blockmap_t;

/* get the blockmap counters */
static size_t *blockmap_counters(blockmap_t *map) {
    return (size_t*)map->data;
}

/* get the blockmap data itself */
static void *blockmap_data(blockmap_t *map) {
    return map->data + (sizeof(size_t) * map->blocks);
}

static void *blockmap_entry(blockmap_t *map, size_t index) {
    return blockmap_data(map) + (map->width * index);
}

static size_t next_free_block(blockmap_t *arena, size_t start) {
    size_t *counters = blockmap_counters(arena);
    for (size_t i = start; i < arena->blocks; i++) {
        if (counters[i] == 0) {
            return i;
        }
    }

    return SIZE_MAX;
}

static size_t more_free_blocks(blockmap_t *arena, size_t start, size_t blocks) {
    size_t *counters = blockmap_counters(arena);

    size_t front = start;
    size_t count = blocks;
    
    for (size_t i = front; i < arena->blocks; i++) {
        if (counters[i] == 0) {
            count -= 1;
        } else {
            front = i;
            count = blocks;
        }

        if (count == 0) {
            return front;
        }
    }

    if (start == 0) {
        return SIZE_MAX;
    }

    return more_free_blocks(arena, 0, blocks);
}

static size_t blockmap_index(blockmap_t *arena, void *ptr) {
    CTASSERTF(ptr >= blockmap_data(arena), "arena [%s] does not contain ptr `%p`", arena->name, ptr);
    return (ptr - blockmap_data(arena)) / arena->width;
}

static void *blockmap_malloc(blockmap_t *arena, size_t bytes) {
    size_t index = next_free_block(arena, 0);
    size_t *counters = blockmap_counters(arena);
    
    CTASSERTF(index != SIZE_MAX, "arena [%s] no free blocks while allocating object of size %zu", arena->name, bytes);

    counters[index] = bytes;
    return blockmap_entry(arena, index);
}

static void blockmap_realloc(blockmap_t *arena, void **ptr, size_t previous, size_t bytes) {
    UNUSED(previous);

    /* get the index of this block in our blockmap */
    size_t index = blockmap_index(arena, *ptr);
    size_t *counters = blockmap_counters(arena);

    /* theres enough space in just this block */
    if (bytes < arena->width) {
        counters[index] = bytes;
        return;
    }

    /* scan for extra space if needed */
    size_t extra_blocks = ALIGN(bytes, arena->width);
    size_t extra_data = more_free_blocks(arena, index, extra_blocks);
    
    /* if theres no extra space then bail */
    if (extra_data == SIZE_MAX) {
        *ptr = NULL;
        return;
    }

    /* otherwise mark the found empty space as used and return it */
    size_t remainder = bytes;
    for (size_t i = extra_data; i < extra_blocks + extra_data; i++) {
        counters[i] = MIN(remainder, arena->width);
        remainder -= arena->width;
    }

    *ptr = blockmap_entry(arena, extra_data);
}

static void blockmap_free(blockmap_t *arena, void *ptr, size_t bytes) {
    size_t index = blockmap_index(arena, ptr);
    size_t *counters = blockmap_counters(arena);
    while (bytes > 0) {
        bytes -= counters[index];
        counters[index] = 0;
        index += 1;
    }
}

arena_t new_blockmap(const char *name, size_t width, size_t blocks) {
    /* reserve enough space for the header, data, and counters */
    size_t bytes = ((width + sizeof(size_t)) * blocks) + sizeof(blockmap_t);
    arena_t result = NEW_ARENA(
        /* name = */ name,
        /* initial = */ bytes,
        /* malloc = */ blockmap_malloc,
        /* realloc = */ blockmap_realloc,
        /* free = */ blockmap_free
    );

    blockmap_t *arena = result.data;
    arena->name = name;
    arena->blocks = blocks;
    arena->width = width;

    size_t *counters = blockmap_counters(arena);
    for (size_t i = 0; i < blocks; i++) {
        counters[i] = 0;
    }

    return result;
}
