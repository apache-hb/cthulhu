#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *formatv(const char *fmt, va_list args) {
    va_list again;
    va_copy(again, args);
    int len = vsnprintf(NULL, 0, fmt, args) + 1;

    char *out = malloc(len);

    vsnprintf(out, len, fmt, again);

    return out;
}

bool startswith(const char *str, const char *other) {
    return strncmp(str, other, strlen(other)) == 0;
}

char *format(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    char *out = formatv(fmt, args);
    va_end(args);

    return out;
}

char *str_replace(const char *str, const char *old, const char *with) {
    size_t slen = strlen(str),
           olen = strlen(old),
           wlen = strlen(with);  

    /**
     * calculate the total size required
     * for the new string
     */
    size_t rlen = slen;
    size_t idx = 0;
    while (idx < slen) {
        if (startswith(str + idx, old)) {
            rlen -= olen;
            rlen += wlen;
            idx += olen;
        } else {
            idx += 1;
        }
    }

    printf("len: %zu\n", rlen);

    char *out = malloc(rlen + 1);
    out[rlen] = 0;

    char *cur = out;

    idx = 0;
    while (cur < (out + rlen)) {
        if (startswith(str + idx, old)) {
            printf("replace\n");
            memcpy(cur, with, wlen);
            cur += wlen;
            idx += olen;
        } else {
            *cur = str[idx];
            cur += 1;
            idx += 1;
        }
    }

    return out;
}

char *str_join(const char *sep, const char **strs, size_t num) {
    size_t slen = strlen(sep);
    size_t len = (num * slen);

    for (size_t i = 0; i < num; i++) {
        len += strlen(strs[i]);
    }

    char *out = malloc(len + 1);
    out[len] = 0;
    
    char *cur = out;

    for (size_t i = 0; i < num; i++) {
        if (i != 0) {
            memcpy(cur, sep, slen);
            cur += slen;
        }
        size_t len = strlen(strs[i]);
        memcpy(cur, strs[i], len);
        cur += len;
    }

    return out;
}

#ifndef _MSC_VER
char *strdup(const char *str) {
    size_t len = strlen(str) + 1;
    char *out = malloc(len);
    memcpy(out, str, len);
    return out;
}
#endif