#include "util.h"

#include "report.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

bool startswith(const char *str, const char *sub) {
    return strncmp(str, sub, strlen(sub)) == 0;
}

char *format(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    char *out = formatv(fmt, args);
    va_end(args);

    return out;
}

char *formatv(const char *fmt, va_list args) {
    va_list again;
    va_copy(again, args);
    int len = vsnprintf(NULL, 0, fmt, args) + 1;

    char *out = malloc(len);

    int err = vsnprintf(out, len, fmt, again);

    ASSERT(err >= 0);

    return out;
}
