#pragma once

#include <ctu_arena_api.h>

#include "base/bitset.h"

#include <limits.h>

typedef struct arena_t arena_t;

CT_BEGIN_API

/// @brief bitmap allocator
typedef struct bitmap_t
{
    /// @brief data for the arena
    FIELD_SIZE(count * stride) char *data;

    /// @brief index of the most recently allocated element
    size_t index;

    // only retain the stride in debug mode for validation
    // in release we assume the size passed to malloc is == stride
#if CTU_DEBUG
    /// @brief size of each element
    size_t stride;
#endif

    /// @brief bitset for the arena
    bitset_t bitset;

    /// @brief memory source
    arena_t *parent;

    /// @brief next bitmap in the chain
    struct bitmap_t *next;
} bitmap_t;

CT_ARENA_API void bitmap_arena_init(arena_t *arena, bitmap_t *bitmap);

CT_ARENA_API bitmap_t *bitmap_new(size_t count, size_t stride, arena_t *arena);
CT_ARENA_API void bitmap_init(bitmap_t *bitmap, size_t count, size_t stride, arena_t *arena);

CT_ARENA_API void *bitmap_alloc(bitmap_t *bitmap, size_t size);
CT_ARENA_API void bitmap_free(bitmap_t *bitmap, void *ptr, size_t size);

CT_ARENA_API void bitmap_reset(bitmap_t *arena);

CT_END_API
