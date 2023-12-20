#include "editor/arena.hpp"
#include "memory/memory.h"

using namespace ed;

static void *wrap_malloc(const mem_t *mem, size_t size)
{
    arena_t *alloc = mem_arena(mem);
    IArena *user = reinterpret_cast<IArena*>(alloc->user);
    return user->malloc(size);
}

static void *wrap_realloc(const mem_t *mem, void *ptr, size_t new_size, size_t old_size)
{
    arena_t *alloc = mem_arena(mem);
    IArena *user = reinterpret_cast<IArena*>(alloc->user);
    return user->realloc(ptr, new_size, old_size);
}

static void wrap_free(const mem_t *mem, void *ptr, size_t size)
{
    arena_t *alloc = mem_arena(mem);
    IArena *user = reinterpret_cast<IArena*>(alloc->user);
    user->free(ptr, size);
}

static void wrap_rename(const mem_t *mem, const void *ptr, const char *name)
{
    arena_t *alloc = mem_arena(mem);
    IArena *user = reinterpret_cast<IArena*>(alloc->user);
    user->set_name(ptr, name);
}

static void wrap_reparent(const mem_t *mem, const void *ptr, const void *parent)
{
    arena_t *alloc = mem_arena(mem);
    IArena *user = reinterpret_cast<IArena*>(alloc->user);
    user->set_parent(ptr, parent);
}

IArena::IArena(const char *alloc_name)
{
    user = this;
    name = alloc_name;
    fn_malloc = wrap_malloc;
    fn_realloc = wrap_realloc;
    fn_free = wrap_free;
    fn_rename = wrap_rename;
    fn_reparent = wrap_reparent;
}

void IArena::install_gmp()
{
    init_gmp_alloc(this);
}
