#include "base/util.h"
#include "base/panic.h"

#include <stdint.h>
#include <string.h>

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
size_t ctu_strlen(const char *text)
{
    CTASSERT(text != NULL);

    return strlen(text);
}

USE_DECL
int ctu_strncmp(const char *lhs, const char *rhs, size_t length)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    return strncmp(lhs, rhs, length);
}

USE_DECL
text_t text_make(char *text, size_t length)
{
    CTASSERT(text != NULL);

    text_t result = { text, length };

    return result;
}

USE_DECL
text_t text_from(char *str)
{
    return text_make(str, ctu_strlen(str));
}

USE_DECL
text_view_t text_view_make(const char *text, size_t length)
{
    CTASSERT(text != NULL);

    text_view_t result = { text, length };

    return result;
}

USE_DECL
text_view_t text_view_from(const char *str)
{
    return text_view_make(str, ctu_strlen(str));
}

bool text_equal(text_view_t lhs, text_view_t rhs)
{
    if (lhs.length != rhs.length)
    {
        return false;
    }

    return ctu_strncmp(lhs.text, rhs.text, lhs.length) == 0;
}
