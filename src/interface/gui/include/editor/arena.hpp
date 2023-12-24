#pragma once

#include "core/macros.h"

#include "memory/arena.h"

namespace ed
{
    struct IArena : public arena_t
    {
        IArena(const char *alloc_name);

        virtual void *malloc(size_t size) = 0;
        virtual void *realloc(void *ptr, size_t new_size, size_t old_size) = 0;
        virtual void free(void *ptr, size_t size) = 0;

        virtual void set_name(const void *ptr, const char *new_name)
        {
            CTU_UNUSED(ptr);
            CTU_UNUSED(new_name);
        }

        virtual void set_parent(const void *ptr, const void *parent)
        {
            CTU_UNUSED(ptr);
            CTU_UNUSED(parent);
        }

        void install_global();
        void install_gmp();
    };
} // namespace ed
