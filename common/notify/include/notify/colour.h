#pragma once

#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

typedef struct text_colour_t text_colour_t;

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

const text_colour_t *colour_get_default(void);
const text_colour_t *colour_get_disabled(void);

const char *colour_get(const text_colour_t *colours, colour_t idx);
const char *colour_reset(const text_colour_t *colours);

END_API
