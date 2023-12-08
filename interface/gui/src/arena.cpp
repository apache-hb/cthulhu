#include "editor/arena.hpp"

using namespace ed;

static void *wrap_malloc(malloc_event_t event)
{
    alloc_t *alloc = event.alloc;
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    return user->malloc(event.size, event.name, event.parent);
}

static void *wrap_realloc(realloc_event_t event)
{
    alloc_t *alloc = event.alloc;
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    return user->realloc(event.ptr, event.new_size, event.old_size);
}

static void wrap_free(free_event_t event)
{
    alloc_t *alloc = event.alloc;
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    user->free(event.ptr, event.size);
}

IAlloc::IAlloc(const char *alloc_name)
{
    user = this;
    name = alloc_name;
    arena_malloc = wrap_malloc;
    arena_realloc = wrap_realloc;
    arena_free = wrap_free;
}
