#include "ctu/util/arena.h"

#include "util.h"

#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const char *name;
    size_t width;
    size_t entries;
    char data[];
} bitmap_t;

static uint8_t *bitmap_bits(bitmap_t *arena) {
    return (uint8_t*)arena->data;
}

static void *bitmap_data(bitmap_t *arena) {
    return arena->data + arena->entries;
}

static void *bitmap_entry(bitmap_t *arena, size_t index) {
    return bitmap_data(arena) + (arena->width * index);
}

static size_t find_block(bitmap_t *arena) {
    uint8_t *bits = bitmap_bits(arena);
    size_t entries = arena->entries;
    
    /* loop over the bitmap entries */
    for (size_t i = 0; i < entries; i++) {
        if (bits[i] == 0xFF) {
            continue;
        }

        /* loop over the bits in the entry */
        for (size_t bit = 0; bit < 8; bit++) {
            if ((bits[i] & (1 << bit)) == 0) {
                return i * 8 + bit;
            }
        }
    }

    return SIZE_MAX;
}

static void mark_used(bitmap_t *arena, size_t block) {
    uint8_t *bits = bitmap_bits(arena);
    bits[block / 8] |= (1 << (block % 8));
}

static void *bitmap_malloc(bitmap_t *arena, size_t bytes) {
    CTASSERTF(arena->width == bytes, "bitmap alloc size mismatch in %s. got size %zu, expected size %zu", bytes, arena->width);

    size_t block = find_block(arena);
    CTASSERTF(block != SIZE_MAX, "bitmap out of memory in %s", arena->name);

    mark_used(arena, block);

    return bitmap_entry(arena, block);
}

static void bitmap_realloc(bitmap_t *arena, void **ptr, size_t previous, size_t bytes) {
    CTASSERTF(false, "reallocating inside bitmap `%s` of pointer %p with size %zu to new size of %zu", arena->name, *ptr, previous, bytes);
}

static void bitmap_free(bitmap_t *arena, void *ptr, size_t bytes) {
    CTASSERTF(arena->width == bytes, "bitmap free size mismatch in %s with ptr %p of size %zu, expected size %zu", ptr, bytes, arena->width);
}

arena_t new_bitmap(const char *name, size_t width, size_t blocks) {
    size_t size = sizeof(bitmap_t) + (blocks * width);
    size_t bitmap = ALIGN(blocks / 8, 8);

    /* allocate enough space for the data and the bitmap */
    size_t aligned = ALIGN4K(bitmap + size); 

    void *data = MMAP_ARENA(aligned);

    bitmap_t *arena = data;
    arena->name = name;
    arena->width = width;
    arena->entries = blocks;

    /* set the bitmap to all 0s */
    uint8_t *bits = bitmap_bits(arena);
    for (size_t i = 0; i < bitmap; i++) {
        bits[i] = 0;
    }

    return NEW_ARENA(name, bitmap_malloc, bitmap_realloc, bitmap_free, arena);
}
