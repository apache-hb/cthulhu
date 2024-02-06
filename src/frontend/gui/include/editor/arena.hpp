#pragma once

#include "core/macros.h"

#include "arena/arena.h"

namespace ed
{
    class IArena : public arena_t
    {
    protected:
        IArena(const char *alloc_name);

    public:
        virtual void *malloc(size_t size) = 0;
        virtual void *realloc(void *ptr, size_t new_size, size_t old_size) = 0;
        virtual void free(void *ptr, size_t size) = 0;

        virtual void set_name(const void *ptr, const char *new_name)
        {
            CT_UNUSED(ptr);
            CT_UNUSED(new_name);
        }

        virtual void set_parent(const void *ptr, const void *new_parent)
        {
            CT_UNUSED(ptr);
            CT_UNUSED(new_parent);
        }

        void install_global();
        void install_gmp();
    };
} // namespace ed
