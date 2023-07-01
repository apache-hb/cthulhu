#include "common.h"

#include "std/str.h"

#include <stdarg.h>
#include <string.h>

void write_string(io_t *io, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = formatv(fmt, args);
    va_end(args);

    io_write(io, msg, strlen(msg));
}
