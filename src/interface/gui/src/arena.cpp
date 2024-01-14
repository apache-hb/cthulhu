#include "editor/arena.hpp"
#include "memory/memory.h"

using namespace ed;

static void *wrap_malloc(size_t size, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    return arena->malloc(size);
}

static void *wrap_realloc(void *ptr, size_t new_size, size_t old_size, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    return arena->realloc(ptr, new_size, old_size);
}

static void wrap_free(void *ptr, size_t size, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    arena->free(ptr, size);
}

static void wrap_rename(const void *ptr, const char *name, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    arena->set_name(ptr, name);
}

static void wrap_reparent(const void *ptr, const void *parent, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    arena->set_parent(ptr, parent);
}

IArena::IArena(const char *alloc_name)
{
    name = alloc_name;
    fn_malloc = wrap_malloc;
    fn_realloc = wrap_realloc;
    fn_free = wrap_free;
    fn_rename = wrap_rename;
    fn_reparent = wrap_reparent;
}

void IArena::install_global()
{
    init_global_arena(this);
}

void IArena::install_gmp()
{
    init_gmp_arena(this);
}
