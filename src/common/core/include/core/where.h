#pragma once

#include <stdint.h>
#include <inttypes.h>

/// @brief a line number
typedef uint_fast64_t line_t;

/// @brief a column number
typedef uint_fast64_t column_t;

/// @brief format specifier for @a line_t
#define PRI_LINE PRIuFAST64

/// @brief format specifier for @a column_t
#define PRI_COLUMN PRIuFAST64

/// @brief a location inside a scanner
/// locations are inclusive and 0-based
typedef struct where_t
{
    /// @brief the first line of the location
    line_t first_line;

    /// @brief the last line of the location
    line_t last_line;

    /// @brief the first column of the location
    column_t first_column;

    /// @brief the last column of the location
    column_t last_column;
} where_t;
