// SPDX-License-Identifier: LGPL-3.0-only

#include "setup/memory.h"
#include "setup/setup2.h"

#include "arena/arena.h"
#include "base/panic.h"
#include "core/macros.h"

#include <stdlib.h>

/// default global allocator

static void *default_malloc(size_t size, void *user)
{
    CT_UNUSED(user);

    return malloc(size);
}

static void *default_realloc(void *ptr, size_t new_size, size_t old_size, void *user)
{
    CT_UNUSED(user);
    CT_UNUSED(old_size);

    return realloc(ptr, new_size);
}

static void default_free(void *ptr, size_t size, void *user)
{
    CT_UNUSED(user);
    CT_UNUSED(size);

    free(ptr);
}

static arena_t gDefaultAlloc = {
    .name = "default global allocator",
    .fn_malloc = default_malloc,
    .fn_realloc = default_realloc,
    .fn_free = default_free
};

arena_t *ctu_default_alloc(void)
{
    return &gDefaultAlloc;
}

/// electric fence allocator

static void *ef_malloc(size_t size, void *self)
{
    CT_UNUSED(self);

    CT_NEVER("bzzt! electric fence hit with malloc of size %zu", size);
}

static void *ef_realloc(void *ptr, size_t new_size, size_t old_size, void *self)
{
    CT_UNUSED(ptr);
    CT_UNUSED(self);

    CT_NEVER("bzzt! electric fence hit with realloc of size %zu old %zu", new_size, old_size);
}

static void ef_free(void *ptr, size_t size, void *self)
{
    CT_UNUSED(ptr);
    CT_UNUSED(self);

    CT_NEVER("bzzt! electric fence hit with free of size %zu", size);
}

static arena_t gElectricFence = {
    .name = "electric fence",
    .fn_malloc = ef_malloc,
    .fn_realloc = ef_realloc,
    .fn_free = ef_free,
};

arena_t *electric_fence_arena(void)
{
    return &gElectricFence;
}
