// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/compiler.h"

CT_BEGIN_API

typedef struct arena_t arena_t;
typedef struct io_t io_t;
typedef struct colour_pallete_t colour_pallete_t;

/// @ingroup format
/// @{

/// @brief line heading style
typedef enum fmt_heading_t
{
#define FMT_HEADING(id, name) id,
#include "format/format.inc"

    eHeadingCount
} fmt_heading_t;

typedef fmt_heading_t heading_style_t;

#if CT_OS_WINDOWS
#   define CT_DEFAULT_HEADER_STYLE eHeadingMicrosoft
#else
#   define CT_DEFAULT_HEADER_STYLE eHeadingGeneric
#endif

/// @brief generic print options
typedef struct fmt_options_t
{
    /// @brief temporary arena
    arena_t *arena;

    /// @brief io buffer
    io_t *io;

    /// @brief colour pallete to use
    const colour_pallete_t *pallete;
} fmt_options_t;

typedef fmt_options_t print_options_t;

/// @}

CT_END_API
