#include "base/util.h"
#include "base/panic.h"

#include <stdint.h>

USE_DECL
size_t ptrhash(const void *ptr)
{
    uintptr_t key = (uintptr_t)ptr;
    key = (~key) + (key << 18);
    key ^= key >> 31;
    key *= 21;
    key ^= key >> 11;
    key += key << 6;
    key ^= key >> 22;
    return key & SIZE_MAX;
}

USE_DECL
text_t text_make(char *text, size_t size)
{
    CTASSERT(text != NULL);

    text_t result = { text, size };

    return result;
}

USE_DECL
text_view_t text_view_make(const char *text, size_t size)
{
    CTASSERT(text != NULL);

    text_view_t result = { text, size };

    return result;
}
