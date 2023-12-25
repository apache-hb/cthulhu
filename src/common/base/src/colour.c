#include "base/colour.h"
#include "base/panic.h"
#include "core/macros.h"

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
