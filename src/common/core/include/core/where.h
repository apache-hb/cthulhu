// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <stdint.h>
#include <inttypes.h>

/// @brief a line number
typedef uint_fast64_t ctu_line_t;

/// @brief a column number
typedef uint_fast64_t ctu_column_t;

/// @brief format specifier for @a ctu_line_t
#define PRI_LINE PRIuFAST64

/// @brief format specifier for @a ctu_column_t
#define PRI_COLUMN PRIuFAST64

/// @brief a location inside a scanner
/// locations are inclusive and 0-based
typedef struct where_t
{
    /// @brief the first line of the location
    ctu_line_t first_line;

    /// @brief the last line of the location
    ctu_line_t last_line;

    /// @brief the first column of the location
    ctu_column_t first_column;

    /// @brief the last column of the location
    ctu_column_t last_column;
} where_t;
