#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdarg.h>

BEGIN_API

typedef struct arena_t arena_t;

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

typedef struct colour_pallete_t
{
    const char *colours[eColourCount];
    const char *reset;
} colour_pallete_t;

extern const colour_pallete_t kColourNone;
extern const colour_pallete_t kColourDefault;

const char *colour_get(const colour_pallete_t *colours, colour_t idx);
const char *colour_reset(const colour_pallete_t *colours);

typedef struct format_context_t
{
    arena_t *arena;
    const colour_pallete_t *pallete;
} format_context_t;

char *colour_text(format_context_t context, colour_t idx, const char *text);

CT_PRINTF(3, 4)
char *colour_format(format_context_t context, colour_t idx, FMT_STRING const char *fmt, ...);
char *colour_vformat(format_context_t context, colour_t idx, const char *fmt, va_list args);

END_API