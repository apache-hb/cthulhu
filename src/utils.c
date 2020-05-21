#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

char* strfmt(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* out = malloc(512);
    snprintf(out, 512, fmt, args);
    va_end(args);

    return out;
}
