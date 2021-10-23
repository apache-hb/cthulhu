#include "str.h"

#include "util.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

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
        if (i != 0) {
            strcpy(out + idx, sep);
            idx += seplen;
        }

        const char *part = vector_get(parts, i);
        strcpy(out + idx, part);
        idx += strlen(part);
    }

    out[idx] = 0;
    return out;
}

char *strmul(const char *str, size_t times) {
    size_t len = strlen(str);
    size_t outlen = len * times;
    char *out = ctu_malloc(outlen + 1);
    for (size_t i = 0; i < times; i++) {
        strcpy(out + i * len, str);
    }
    out[outlen] = 0;
    return out;
}

static bool ctu_isprint(char c) {
    return isprint(c) || c == 0x0A;
}

char *strnorm(const char *str) {
    size_t len = 0;
    const char *temp = str;
    while (*temp != '\0') {
        len += (ctu_isprint(*temp) ? 1 : 4);
        temp += 1;
    }

    char *buf = ctu_malloc(len + 1);
    char *out = buf;
    while (*str != '\0') {
        if (ctu_isprint(*str)) {
            *out++ = *str;
        } else {
            out += sprintf(out, "\\x%02x", *str);
        }
        str += 1;
    }
    *out = '\0';

    return buf;
}

char *nstrnorm(const char *str, size_t len) {
    size_t outlen = 1;
    for (size_t i = 0; i < len; i++) {
        outlen += (ctu_isprint(str[i]) ? 1 : 4);
    }

    char *buf = ctu_malloc(outlen + 1);
    char *out = buf;
    for (size_t i = 0; i < len; i++) {
        if (ctu_isprint(str[i])) {
            *out++ = str[i];
        } else {
            out += sprintf(out, "\\x%02x", str[i] & 0xFF);
        }
    }

    *out = '\0';
    return buf;
}

vector_t *strsplit(const char *str, const char *sep) {
    vector_t *result = vector_new(4);
    char *save;
    char *token = strtok_r((char *)str, sep, &save);
    while (token != NULL) {
        vector_push(&result, token);
        token = strtok_r(NULL, sep, &save);
    }

    return result;
}

size_t strhash(const char *str) {
    size_t hash = 0;

    while (*str) {
        // hash = 31 * hash + (*str++ & 0xff);
        hash = (hash << 5) - hash + *str++;
    }

    return hash;
}

bool streq(const char *lhs, const char *rhs) {
    /* compare pointers as well for better perf
       with interned strings */
    return lhs == rhs || strcmp(lhs, rhs) == 0;
}


char *strslice(const char *str, size_t start, size_t end) {
    size_t bytes = end - start;
    return ctu_strndup(str + start, bytes);
}

stream_t *stream_new(size_t size) {
    stream_t *out = ctu_malloc(sizeof(stream_t));
    out->size = size;
    out->len = 0;

    out->data = ctu_malloc(size + 1);
    out->data[0] = 0;

    return out;
}

void stream_delete(stream_t *stream) {
    ctu_free(stream->data);
    ctu_free(stream);
}

size_t stream_len(stream_t *stream) {
    return stream->len;
}

void stream_write(stream_t *stream, const char *str) {
    size_t len = strlen(str);
    if (stream->len + len > stream->size) {
        stream->size = stream->len + len;
        stream->data = ctu_realloc(stream->data, stream->size + 1);
    }

    strcpy(stream->data + stream->len, str);
    stream->len += len;
}

const char *stream_data(const stream_t *stream) {
    return stream->data;
}

/**
 * modified version of https://rosettacode.org/wiki/Longest_common_prefix#C
 * 
 * expects a list of file paths
 */
const char *common_prefix(vector_t *args) {
    size_t len = vector_len(args);
    char **strings = ctu_malloc(len * sizeof(char*));

    size_t min = SIZE_MAX;

    for (size_t i = 0; i < len; i++) {
        char *arg = vector_get(args, i);
        size_t len = rfind(arg, '/');
        strings[i] = ctu_strndup(arg, len);

        min = MIN(min, len);
    }

    if (min == 0) {
        return "";
    }

    for (size_t i = 0; i < min; i++) {
        for (size_t j = 1; j < len; j++) {
            if (strings[j][i] != strings[0][i]) {
                if (i == 0) {
                    return "";
                } else {
                    return ctu_strndup(strings[0], i);
                }
            }
        }
    }

    return ctu_strndup(strings[0], min);
}

size_t rfind(const char *str, char c) {
    size_t len = strlen(str);
    while (len--) {
        if (str[len] == c) {
            return len;
        }
    }

    return SIZE_MAX;
}
