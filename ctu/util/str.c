#include "str.h"

#include "util.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *str = formatv(fmt, args);
    va_end(args);

    return str;
}

char *formatv(const char *fmt, va_list args) {
    /* make a copy of the args for the second format */
    va_list again;
    va_copy(again, args);

    /* get the number of bytes needed to format */
    int len = vsnprintf(NULL, 0, fmt, args) + 1;

    char *out = ctu_malloc(len);

    vsnprintf(out, len, fmt, again);

    return out;
}

bool startswith(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool endswith(const char *str, const char *suffix) {
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) {
        return false;
    }

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

char *strjoin(const char *sep, vector_t *parts) {
    size_t all = vector_len(parts);
    size_t len = 0;
    size_t seplen = strlen(sep);
    for (size_t i = 0; i < all; i++) {
        const char *part = vector_get(parts, i);
        len += strlen(part);

        if (i != 0) {
            len += seplen;
        }
    }

    char *out = ctu_malloc(len + 1);
    size_t idx = 0;

    for (size_t i = 0; i < all; i++) {
        const char *part = vector_get(parts, i);
        strcpy(out, part);
        out += strlen(part);

        if (i != 0) {
            strcpy(out + idx, sep);
            idx += seplen;
        }
    }

    out[idx] = 0;
    return out;
}
