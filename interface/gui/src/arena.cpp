#include "editor/arena.hpp"
#include "memory/memory.h"

using namespace ed;

static void *wrap_malloc(const mem_t *mem, malloc_event_t event)
{
    alloc_t *alloc = mem_arena(mem);
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    return user->malloc(event.size, event.name, event.parent);
}

static void *wrap_realloc(const mem_t *mem, realloc_event_t event)
{
    alloc_t *alloc = mem_arena(mem);
    IAlloc *user = reinterpret_cast<IAlloc*>(alloc->user);
    return user->realloc(event.ptr, event.new_size, event.old_size);
}

static void wrap_free(const mem_t *mem, free_event_t event)
{
    alloc_t *alloc = mem_arena(mem);
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

void IAlloc::install()
{
    alloc_t self = *this;
    gDefaultAlloc = self;
    init_gmp(&gDefaultAlloc);
}
