#pragma once

#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

typedef enum colour_t
{
    eColourRed,
    eColourGreen,
    eColourYellow,
    eColourBlue,
    eColourMagenta,
    eColourCyan,
    eColourWhite,
    eColourDefault,

    eColourCount
} colour_t;

// TODO: rework colours to be more flexible

/// @brief a colour pallete
typedef struct text_colour_t
{
    /// @brief the colour set
    const char *colours[eColourCount];

    /// @brief the reset colour
    const char *reset;
} text_colour_t;

extern const text_colour_t kDisabledColour;
extern const text_colour_t kDefaultColour;

const char *colour_get(const text_colour_t *colours, colour_t idx);
const char *colour_reset(const text_colour_t *colours);

END_API
