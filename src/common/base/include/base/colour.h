#pragma once

#include "core/compiler.h"

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

    eColourCount
} colour_t;

typedef struct colour_pallete_t
{
    const char *colours[eColourCount];
    const char *reset;
} colour_pallete_t;

extern const colour_pallete_t kColourNone;
extern const colour_pallete_t kColourDefault;

const char *colour_get(const colour_pallete_t *colours, colour_t idx);
const char *colour_reset(const colour_pallete_t *colours);

END_API
