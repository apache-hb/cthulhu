#include "common.h"
#include "memory/arena.h"
#include "std/str.h"
#include <stdarg.h>
#include <string.h>

char *fmt_align(arena_t *arena, size_t width, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = vformat(fmt, args);
    va_end(args);

    size_t len = strlen(msg);
    if (len >= width) return msg;

    size_t size = width - 1;
    char *result = ARENA_MALLOC(arena, size, msg, NULL);
    memset(result, ' ', width);
    memcpy(result, msg, len);

    result[width] = '\0';

    return result;
}
