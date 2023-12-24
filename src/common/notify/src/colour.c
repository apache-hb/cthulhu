#include "notify/colour.h"

#include "base/panic.h"

#include "core/macros.h"

/// @brief a colour pallete
typedef struct text_colour_t
{
    /// @brief the colour set
    const char *colours[eColourCount];

    /// @brief the reset colour
    const char *reset;
} text_colour_t;

const text_colour_t kDisabledColour = {
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

    .reset = "",
};

const text_colour_t kDefaultColour = {
    .colours = {
        [eColourRed] = ANSI_RED,
        [eColourGreen] = ANSI_GREEN,
        [eColourYellow] = ANSI_YELLOW,
        [eColourBlue] = ANSI_BLUE,
        [eColourMagenta] = ANSI_MAGENTA,
        [eColourCyan] = ANSI_CYAN,
        [eColourWhite] = ANSI_WHITE,
        [eColourDefault] = ANSI_DEFAULT,
    },

    .reset = ANSI_RESET,
};

const text_colour_t *colour_get_default(void)
{
    return &kDefaultColour;
}

const text_colour_t *colour_get_disabled(void)
{
    return &kDisabledColour;
}

const char *colour_get(const text_colour_t *colours, colour_t idx)
{
    CTASSERT(colours != NULL);
    CTASSERT(idx < eColourCount);

    return colours->colours[idx];
}

const char *colour_reset(const text_colour_t *colours)
{
    CTASSERT(colours != NULL);

    return colours->reset;
}
