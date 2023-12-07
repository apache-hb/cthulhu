#include "editor/alloc.h"

static void *wrap_malloc(alloc_t *alloc, size_t size, const char *name, const void *parent)
{
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    return user->malloc(size, name, parent);
}

static void *wrap_realloc(alloc_t *alloc, void *ptr, size_t new_size, size_t old_size)
{
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    return user->realloc(ptr, new_size, old_size);
}

static void wrap_free(alloc_t *alloc, void *ptr, size_t size)
{
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    user->free(ptr, size);
}

IAlloc::IAlloc(const char *alloc_name)
{
    user = this;
    name = alloc_name;
    arena_malloc = wrap_malloc;
    arena_realloc = wrap_realloc;
    arena_free = wrap_free;
}
