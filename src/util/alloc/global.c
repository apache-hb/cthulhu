#include "cthulhu/util/alloc.h"

#include "cthulhu/util/util.h"

static void *global_malloc(void *self, size_t size)
{
    UNUSED(self);
    return ctu_malloc(size);
}

static void *global_realloc(void *self, void *ptr, size_t newSize, size_t oldSize)
{
    UNUSED(self);
    UNUSED(oldSize);

    return ctu_realloc(ptr, newSize);
}

static void global_free(void *self, void *ptr, size_t size)
{
    UNUSED(self);
    UNUSED(size);

    ctu_free(ptr);
}

static alloc_t kGlobalAlloc = {
    .malloc = global_malloc,
    .realloc = global_realloc,
    .free = global_free,

    .state = NULL
};

alloc_t *alloc_global(void)
{
    return &kGlobalAlloc;
}
