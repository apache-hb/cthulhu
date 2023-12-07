#pragma once

#include "memory/arena.h"

struct IAlloc : public alloc_t
{
    IAlloc(const char *alloc_name);

    virtual void *malloc(size_t size, const char *name, const void *parent) = 0;
    virtual void *realloc(void *ptr, size_t new_size, size_t old_size) = 0;
    virtual void free(void *ptr, size_t size) = 0;
};
