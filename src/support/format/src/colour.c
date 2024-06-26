// SPDX-License-Identifier: LGPL-3.0-only

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
        [eColourDefault] = "",
    },
    .reset = ""
};

const colour_pallete_t kColourDefault = {
    .colours = {
        [eColourRed] = CT_ANSI_RED,
        [eColourGreen] = CT_ANSI_GREEN,
        [eColourYellow] = CT_ANSI_YELLOW,
        [eColourBlue] = CT_ANSI_BLUE,
        [eColourMagenta] = CT_ANSI_MAGENTA,
        [eColourCyan] = CT_ANSI_CYAN,
        [eColourWhite] = CT_ANSI_WHITE,
        [eColourDefault] = "",
    },
    .reset = CT_ANSI_RESET
};

STA_DECL
const char *colour_get(const colour_pallete_t *colours, colour_t idx)
{
    CTASSERT(colours != NULL);
    CTASSERT(idx < eColourCount);

    return colours->colours[idx];
}

STA_DECL
const char *colour_reset(const colour_pallete_t *colours)
{
    CTASSERT(colours != NULL);

    return colours->reset;
}

STA_DECL
char *colour_text(format_context_t context, colour_t idx, const char *text)
{
    const char *colour = colour_get(context.pallete, idx);
    const char *reset = colour_reset(context.pallete);

    return str_format(context.arena, "%s%s%s", colour, text, reset);
}

STA_DECL
char *colour_format(format_context_t context, colour_t idx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = colour_vformat(context, idx, fmt, args);
    va_end(args);

    return msg;
}

STA_DECL
char *colour_vformat(format_context_t context, colour_t idx, const char *fmt, va_list args)
{
    const char *colour = colour_get(context.pallete, idx);
    const char *reset = colour_reset(context.pallete);

    char *msg = str_vformat(context.arena, fmt, args);

    return str_format(context.arena, "%s%s%s", colour, msg, reset);
}
