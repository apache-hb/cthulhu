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

char *strdup(const char *str) {
    size_t len = strlen(str) + 1;
    char *out = malloc(len);
    memcpy(out, str, len);
    return out;
}
