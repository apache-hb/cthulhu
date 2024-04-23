// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/analyze.h"

#include <stddef.h>
#include <stdint.h>

/// @defgroup source_info Source location information
/// @brief Source location information for panics and other debugging information
/// @ingroup core
/// @{

/// @brief the line number in the source file
typedef uint_fast32_t source_line_t;

/// @brief format specifier for @a source_line_t
#define CT_PRI_LINE PRIuFAST32

/// @brief a line number that is unknown
#define CT_LINE_UNKNOWN UINT_FAST32_MAX

/// @brief panic location information
typedef struct source_info_t
{
    /// @brief the file the panic occurred in
    FIELD_STRING const char *file;

    /// @brief the line the panic occurred on
    source_line_t line;

    /// @brief the function the panic occurred in
    /// @note this could also be the name of a variable in c++ depending on context
    FIELD_STRING const char *function;
} source_info_t;

/// @def CT_SOURCE_CURRENT
/// @brief the source location of the current line
#define CT_SOURCE_CURRENT {__FILE__, __LINE__, CT_FUNCTION_NAME}

/// @}
