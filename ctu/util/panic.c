#include "macros.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

void ctpanic(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(99);
}
