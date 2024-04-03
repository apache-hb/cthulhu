// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "arena/arena.h"

#include "editor/utils/utils.hpp"

class TraceArena;

class IArena
{
    arena_t impl;

    static void *wrap_malloc(size_t size, void *user);
    static void *wrap_realloc(void *ptr, size_t new_size, size_t old_size, void *user);
    static void wrap_free(void *ptr, size_t size, void *user);
    static void wrap_rename(const void *ptr, const char *name, void *user);
    static void wrap_reparent(const void *ptr, const void *parent, void *user);

protected:
    IArena(const char *id);

public:
    virtual ~IArena() = default;

    arena_t *get_arena() { return &impl; }
    const char *get_name() const { return impl.name; }

    virtual void *malloc(size_t size) = 0;
    virtual void *realloc(void *ptr, size_t new_size, size_t old_size) = 0;
    virtual void free(void *ptr, size_t size) = 0;

    virtual void rename(CTX_UNUSED const void *ptr, CTX_UNUSED const char *new_name) { }
    virtual void reparent(CTX_UNUSED const void *ptr, CTX_UNUSED const void *new_parent) { }
};
