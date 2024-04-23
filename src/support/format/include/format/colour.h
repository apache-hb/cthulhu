// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_format_api.h>

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdarg.h>

CT_BEGIN_API

typedef struct arena_t arena_t;

/// @defgroup colour Terminal colours
/// @ingroup format
/// @brief Terminal colours for formatting
/// @{

/// @brief a colour code
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
typedef struct colour_pallete_t
{
    const char *colours[eColourCount];
    FIELD_STRING const char *reset;
} colour_pallete_t;

/// @brief a formatting context when using colours
typedef struct format_context_t
{
    arena_t *arena;
    const colour_pallete_t *pallete;
} format_context_t;

/// @brief a colour pallete that applies no colours
CT_FORMAT_API extern const colour_pallete_t kColourNone;

/// @brief a colour pallete that applies ANSI VT100 colours
CT_FORMAT_API extern const colour_pallete_t kColourDefault;

/// @brief get a colours string form from a pallete
///
/// @param colours the pallete to get the colour from
/// @param idx the colour to get
///
/// @return the colour string
STA_RET_STRING CT_PUREFN
CT_FORMAT_API const char *colour_get(IN_NOTNULL const colour_pallete_t *colours, IN_DOMAIN(<, eColourCount) colour_t idx);

/// @brief get a reset string from a pallete
///
/// @param colours the pallete to get the reset from
///
/// @return the reset string
STA_RET_STRING CT_PUREFN
CT_FORMAT_API const char *colour_reset(IN_NOTNULL const colour_pallete_t *colours);

/// @brief add colour to a string
///
/// @param context the context to use
/// @param idx the colour to add
/// @param text the text to colour
///
/// @return the coloured string
STA_RET_STRING
CT_FORMAT_API char *colour_text(
    format_context_t context,
    IN_DOMAIN(<, eColourCount) colour_t idx,
    IN_STRING const char *text);

/// @brief format a string and add colour to it
///
/// @param context the context to use
/// @param idx the colour to add
/// @param fmt the format string
/// @param ... the format arguments
///
/// @return the coloured string
STA_RET_STRING STA_PRINTF(3, 4)
CT_FORMAT_API char *colour_format(
    format_context_t context,
    IN_DOMAIN(<, eColourCount) colour_t idx,
    STA_FORMAT_STRING const char *fmt, ...);

/// @brief format a string and add colour to it
///
/// @param context the context to use
/// @param idx the colour to add
/// @param fmt the format string
/// @param args the format arguments
///
/// @return the coloured string
STA_RET_STRING
CT_FORMAT_API char *colour_vformat(
    format_context_t context,
    IN_DOMAIN(<, eColourCount) colour_t idx,
    IN_STRING const char *fmt, va_list args);

/// @}

CT_END_API
