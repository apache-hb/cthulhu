#include "cthulhu/util/alloc.h"

#include "cthulhu/util/macros.h"

typedef struct 
{
    char *base;
    size_t offset;
    size_t total;
} bump_t;

//
// bump allocators are stored in the provided memory pool.
// such that
//
// alloc_t | bump_t | allocs...
//

void *bump_malloc(void *self, size_t size)
{
    bump_t *bump = self;
    if (bump->offset + size > bump->total)
    {
        return NULL; // out of memory
    }

    void *ptr = bump->base + bump->offset;
    bump->offset += size;
    return ptr;
}

void *bump_realloc(void *self, void *ptr, size_t newSize, size_t oldSize)
{
    UNUSED(ptr);
    UNUSED(oldSize);

    bump_t *bump = self;
    // TODO: support the case where ptr was the most recent allocation

    if (bump->offset + newSize > bump->total)
    {
        return NULL; // out of memory
    }

    void *newPtr = bump->base + bump->offset;
    bump->offset += newSize;
    return newPtr;
}

alloc_t *alloc_bump(void *base, size_t size)
{
    size_t minimumSize = sizeof(alloc_t) + sizeof(bump_t);
    if (size < minimumSize)
    {
        return NULL;
    }
    
    char *ptr = base;

    alloc_t *alloc = base;
    bump_t *bump = (void*)(ptr + sizeof(alloc_t));

    alloc->malloc = bump_malloc;
    alloc->realloc = bump_realloc;
    alloc->free = NULL; // TODO: support freeing most recent allocation
    
    bump->base = ptr;
    bump->offset = 0;
    bump->total = size - minimumSize;

    return alloc;
}
