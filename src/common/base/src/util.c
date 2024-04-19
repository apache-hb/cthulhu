// SPDX-License-Identifier: LGPL-3.0-only

#include "base/util.h"
#include "base/panic.h"

#include <stdint.h>
#include <string.h>

USE_DECL
bool is_path_special(const char *path)
{
    return path == NULL || str_equal(path, ".") || str_equal(path, "..");
}

USE_DECL
ctu_hash_t ctu_ptrhash(const void *ptr)
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
ctu_hash_t str_hash(const char *str)
{
    CTASSERT(str != NULL);

    size_t hash = 0;

    while (*str)
    {
        hash = (hash << 5) - hash + *str++;
    }

    return hash;
}

USE_DECL
ctu_hash_t text_hash(text_view_t text)
{
    CTASSERT(text.text != NULL);

    ctu_hash_t hash = 0;
    for (size_t i = 0; i < text.length; i++)
    {
        hash = (hash << 5) - hash + text.text[i];
    }

    return hash;
}

USE_DECL
bool str_equal(const char *lhs, const char *rhs)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    /* compare pointers as well for better perf
       with interned strings */
    return lhs == rhs || strcmp(lhs, rhs) == 0;
}

USE_DECL
size_t ctu_strlen(const char *str)
{
    CTASSERT(str != NULL);

    return strlen(str);
}

USE_DECL
bool ctu_string_empty(const char *str)
{
    CTASSERT(str != NULL);

    return *str == '\0';
}

USE_DECL
int ctu_strncmp(const char *lhs, const char *rhs, size_t length)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    return strncmp(lhs, rhs, length);
}

USE_DECL
int ctu_strcmp(const char *lhs, const char *rhs)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    return strcmp(lhs, rhs);
}

USE_DECL
char *ctu_strcpy(char *dst, const char *src, size_t size)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    return strncpy(dst, src, size);
}

USE_DECL CT_NOALIAS
void *ctu_memcpy(void *restrict dst, const void *restrict src, size_t size)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    return memcpy(dst, src, size);
}

USE_DECL
void *ctu_memmove(void *dst, const void *src, size_t size)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    return memmove(dst, src, size);
}

USE_DECL CT_NOALIAS
void ctu_memset(void *dst, int value, size_t size)
{
    CTASSERT(dst != NULL);

    memset(dst, value, size);
}

USE_DECL
char *ctu_strstr(IN_STRING const char *haystack, IN_STRING const char *needle)
{
    CTASSERT(haystack != NULL);
    CTASSERT(needle != NULL);

    return strstr(haystack, needle);
}

USE_DECL
text_t text_make(char *text, size_t length)
{
    CTASSERT(text != NULL);

    text_t result = { text, length };

    return result;
}

USE_DECL
text_t text_from(char *text)
{
    return text_make(text, ctu_strlen(text));
}

USE_DECL
text_view_t text_view_make(const char *text, size_t length)
{
    CTASSERT(text != NULL);

    text_view_t result = { text, length };

    return result;
}

USE_DECL
text_view_t text_view_from(const char *text)
{
    return text_view_make(text, ctu_strlen(text));
}

bool text_equal(text_view_t lhs, text_view_t rhs)
{
    if (lhs.length != rhs.length)
    {
        return false;
    }

    return ctu_strncmp(lhs.text, rhs.text, lhs.length) == 0;
}
