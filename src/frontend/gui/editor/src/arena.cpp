// SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.hpp" // IWYU pragma: keep

#include "editor/arena.hpp"

using namespace ed;

void *IArena::wrap_malloc(size_t size, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    return arena->malloc(size);
}

void *IArena::wrap_realloc(void *ptr, size_t new_size, size_t old_size, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    return arena->realloc(ptr, new_size, old_size);
}

void IArena::wrap_free(void *ptr, size_t size, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    arena->free(ptr, size);
}

void IArena::wrap_rename(const void *ptr, const char *name, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    arena->set_name(ptr, name);
}

void IArena::wrap_reparent(const void *ptr, const void *parent, void *user)
{
    IArena *arena = reinterpret_cast<IArena*>(user);
    arena->set_parent(ptr, parent);
}

IArena::IArena(const char *id)
{
    name = id;
    fn_malloc = wrap_malloc;
    fn_realloc = wrap_realloc;
    fn_free = wrap_free;
    fn_rename = wrap_rename;
    fn_reparent = wrap_reparent;
    user = this;
}
