#pragma once

#include "core/analyze.h"

#include <stddef.h>

/// @defgroup source_info Source location information
/// @brief Source location information for panics and other debugging information
/// @ingroup core
/// @{

/// @brief panic location information
typedef struct source_info_t
{
    /// @brief the file the panic occurred in
    FIELD_STRING const char *file;

    /// @brief the line the panic occurred on
    size_t line;

    /// @brief the function the panic occurred in
    /// @note this could also be the name of a variable in some uses in c++
    FIELD_STRING const char *function;
} source_info_t;

/// @def CT_SOURCE_CURRENT
/// @brief the source location of the current line
#define CT_SOURCE_CURRENT {__FILE__, __LINE__, CT_FUNCNAME}

/// @}
