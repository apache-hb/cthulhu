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

static size_t bitmap_index(bitmap_t *arena, void *ptr) {
    CTASSERTF(ptr >= bitmap_data(arena), "arena [%s] does not contain ptr `%p`", arena->name, ptr);
    return (ptr - bitmap_data(arena)) / arena->width;
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

static void mark_bit(bitmap_t *arena, size_t block, bool used) {
    uint8_t *bits = bitmap_bits(arena);
    if (used) {
        bits[block / 8] |= (1 << (block % 8));
    } else {
        bits[block / 8] &= ~(1 << (block % 8));
    }
}

static void *bitmap_malloc(bitmap_t *arena, size_t bytes) {
    CTASSERTF(arena->width == bytes, "bitmap [%s] alloc size mismatch. got size %zu, expected size %zu", bytes, arena->width);

    size_t block = find_block(arena);
    CTASSERTF(block != SIZE_MAX, "bitmap [%s] out of memory", arena->name);

    mark_bit(arena, block, true);

    return bitmap_entry(arena, block);
}

static void bitmap_realloc(bitmap_t *arena, void **ptr, size_t previous, size_t bytes) {
    CTASSERTF(false, "bitmap [%s] reallocating of pointer %p with size %zu to new size of %zu", arena->name, *ptr, previous, bytes);
}

static void bitmap_free(bitmap_t *arena, void *ptr, size_t bytes) {
    CTASSERTF(arena->width == bytes, "bitmap [%s] free size mismatch with ptr %p of size %zu, expected size %zu", ptr, bytes, arena->width);
    size_t index = bitmap_index(arena, ptr);
    mark_bit(arena, index, false);
}

arena_t new_bitmap(const char *name, size_t width, size_t blocks) {
    size_t size = sizeof(bitmap_t) + (blocks * width);
    size_t bitmap = ALIGN(blocks / 8, 8);

    arena_t result = NEW_ARENA(
        /* name = */ name,
        /* initial = */ bitmap + size,
        /* malloc = */ bitmap_malloc,
        /* realloc = */ bitmap_realloc,
        /* free = */ bitmap_free
    );

    bitmap_t *arena = result.data;
    arena->name = name;
    arena->width = width;
    arena->entries = blocks;

    /* set the bitmap to all 0s */
    uint8_t *bits = bitmap_bits(arena);
    for (size_t i = 0; i < bitmap; i++) {
        bits[i] = 0;
    }

    return result;
}
