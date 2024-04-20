// SPDX-License-Identifier: LGPL-3.0-only

#include "setup/memory.h"
#include "setup/setup.h"

#include "arena/arena.h"
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
