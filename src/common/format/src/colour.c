#include "format/colour.h"
#include "base/panic.h"

#include "core/macros.h"

#include "std/str.h"

const colour_pallete_t kColourNone = {
    .colours = {
        [eColourRed] = "",
        [eColourGreen] = "",
        [eColourYellow] = "",
        [eColourBlue] = "",
        [eColourMagenta] = "",
        [eColourCyan] = "",
        [eColourWhite] = "",
    },
    .reset = ""
};

const colour_pallete_t kColourDefault = {
    .colours = {
        [eColourRed] = ANSI_RED,
        [eColourGreen] = ANSI_GREEN,
        [eColourYellow] = ANSI_YELLOW,
        [eColourBlue] = ANSI_BLUE,
        [eColourMagenta] = ANSI_MAGENTA,
        [eColourCyan] = ANSI_CYAN,
        [eColourWhite] = ANSI_WHITE,
    },
    .reset = ANSI_RESET
};

const char *colour_get(const colour_pallete_t *colours, colour_t idx)
{
    CTASSERT(colours != NULL);
    CTASSERT(idx < eColourCount);

    return colours->colours[idx];
}

const char *colour_reset(const colour_pallete_t *colours)
{
    CTASSERT(colours != NULL);

    return colours->reset;
}

char *colour_text(format_context_t context, colour_t idx, const char *text)
{
    const char *colour = colour_get(context.pallete, idx);
    const char *reset = colour_reset(context.pallete);

    return str_format(context.arena, "%s%s%s", colour, text, reset);
}

char *colour_format(format_context_t context, colour_t idx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = colour_vformat(context, idx, fmt, args);
    va_end(args);

    return msg;
}

char *colour_vformat(format_context_t context, colour_t idx, const char *fmt, va_list args)
{
    const char *colour = colour_get(context.pallete, idx);
    const char *reset = colour_reset(context.pallete);

    char *msg = vformat(fmt, args);

    return str_format(context.arena, "%s%s%s", colour, msg, reset);
}
