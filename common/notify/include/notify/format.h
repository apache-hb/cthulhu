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

/// @brief a colour pallete
typedef struct text_colour_t
{
    /// @brief the colour set
    const char *colours[eColourCount];

    /// @brief the reset colour
    const char *reset;
} text_colour_t;

/// @brief the configuration for a file
typedef struct file_config_t
{
    /// @brief the zeroth line of a file is the first line
    bool zeroth_line;
} file_config_t;

extern const text_colour_t kDisabledColour;
extern const text_colour_t kDefaultColour;

END_API
