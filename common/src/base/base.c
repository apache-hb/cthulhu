#include "base/util.h"

#include "base/memory.h"
#include "base/panic.h"
#include "base/macros.h"

#include <gmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

USE_DECL
void *ctu_memdup(const void *ptr, size_t size)
{
    void *out = ctu_malloc(size);
    memcpy(out, ptr, size);
    return out;
}

USE_DECL
char *ctu_strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *out = ctu_malloc(len);
    memcpy(out, str, len);
    return out;
}

USE_DECL
char *ctu_strndup(const char *str, size_t len)
{
    char *out = ctu_malloc(len + 1);
    memcpy(out, str, len);
    out[len] = '\0';
    return out;
}

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
