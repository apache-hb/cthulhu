#include "cthulhu/util/alloc.h"

#include "cthulhu/util/macros.h"

void *alloc_new(alloc_t *alloc, size_t size, const char *name, const void *parent)
{
    UNUSED(name);
    UNUSED(parent);

    return alloc->malloc(alloc->state, size);
}

void *alloc_resize(alloc_t *alloc, void *ptr, size_t newSize, size_t oldSize, const char *name, const void *parent)
{
    UNUSED(name);
    UNUSED(parent);
    
    return alloc->realloc(alloc->state, ptr, newSize, oldSize);
}

void alloc_delete(alloc_t *alloc, void *ptr, size_t size)
{
    CTASSERT(alloc->free != NULL, "allocator does not support freeing");
    alloc->free(alloc->state, ptr, size);
}
