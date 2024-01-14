#pragma once

#include "core/compiler.h"

BEGIN_API

typedef struct arena_t arena_t;
typedef struct io_t io_t;
typedef struct colour_pallete_t colour_pallete_t;

/// @ingroup format
/// @{

/// @brief line heading style
typedef enum heading_style_t
{
    /// @brief generic style
    /// ie: `file:line:col:`
    eHeadingGeneric,

    /// @brief microsoft msvc style
    /// ie: `file(line:col):`
    eHeadingMicrosoft,

    eHeadingCount
} heading_style_t;

/// @brief generic print options
typedef struct print_options_t
{
    /// @brief temporary arena
    arena_t *arena;

    /// @brief io buffer
    io_t *io;

    /// @brief colour pallete to use
    const colour_pallete_t *pallete;
} print_options_t;

/// @}

END_API
