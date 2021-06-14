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
    int len = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char *out = malloc(len);

    va_start(args, fmt);
    int err = vsnprintf(out, len, fmt, args);
    va_end(args);

    ASSERT(err >= 0);
    return out;
}
