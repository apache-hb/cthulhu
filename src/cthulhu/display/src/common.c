#include "common.h"
#include "std/str.h"

char *fmt_coloured2(const colour_pallete_t *colours, colour_t idx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = vformat(fmt, args);
    va_end(args);

    return format("%s%s%s", colour_get(colours, idx), msg, colour_reset(colours));
}
