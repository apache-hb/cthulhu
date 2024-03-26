// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "arena/arena.h"

#define CTX_UNUSED [[maybe_unused]]

namespace ed
{
    class IArena : arena_t
    {
        static void *wrap_malloc(size_t size, void *user);
        static void *wrap_realloc(void *ptr, size_t new_size, size_t old_size, void *user);
        static void wrap_free(void *ptr, size_t size, void *user);
        static void wrap_rename(const void *ptr, const char *name, void *user);
        static void wrap_reparent(const void *ptr, const void *parent, void *user);

        virtual void *malloc(size_t size) = 0;
        virtual void *realloc(void *ptr, size_t new_size, size_t old_size) = 0;
        virtual void free(void *ptr, size_t size) = 0;

        virtual void set_name(CTX_UNUSED const void *ptr, CTX_UNUSED const char *new_name) { }
        virtual void set_parent(CTX_UNUSED const void *ptr, CTX_UNUSED const void *new_parent) { }

    protected:
        IArena(const char *id);

    public:
        arena_t *get_arena() { return this; }
        const char *get_name() const { return name; }
    };
} // namespace ed
