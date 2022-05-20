#include "cthulhu/util/alloc.h"

#include "cthulhu/util/macros.h"

static void *rusty_malloc(void *self, size_t size)
{
    alloc_t *alloc = self;
    void *result = alloc->malloc(alloc->state, size);
    CTASSERT(result != NULL, "malloc failed");
    return result;
}

static void *rusty_realloc(void *self, void *ptr, size_t newSize, size_t oldSize)
{
    alloc_t *alloc = self;
    void *result = alloc->realloc(alloc->state, ptr, newSize, oldSize);
    CTASSERT(result != NULL, "realloc failed");
    return result;
}

static void rusty_free(void *self, void *ptr, size_t size)
{
    alloc_t *alloc = self;
    alloc->free(alloc->state, ptr, size);
}

alloc_t *alloc_rusty(alloc_t *alloc)
{
    alloc_t *rusty = alloc_new(alloc, sizeof(alloc_t), "rusty", NULL);
    CTASSERT(rusty != NULL, "failed to allocate rusty allocator");

    rusty->malloc = rusty_malloc;
    rusty->realloc = rusty_realloc;
    rusty->free = rusty_free;

    rusty->state = alloc;

    return rusty;
}
