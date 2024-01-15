#pragma once

#include <ctu_scan_api.h>

#include "core/analyze.h"

#include "scan/scan.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

BEGIN_API

/// @defgroup location Source location tracking
/// @brief AST source position tracking
/// @ingroup common
/// @{

/// @brief a line number
typedef uint_fast64_t line_t;

/// @brief a column number
typedef uint_fast64_t column_t;

/// @brief a position in a source file
typedef struct node_t node_t;

/// @brief format specifier for @a line_t
#define PRI_LINE PRIuFAST64

/// @brief format specifier for @a column_t
#define PRI_COLUMN PRIuFAST64

/// @brief a location inside a scanner
/// locations are inclusive and 0-based
typedef struct
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

/// @brief get the associated source file of a node
///
/// @param node the node to get the source file of
///
/// @return the source file of @p node
NODISCARD CONSTFN
CT_SCAN_API const scan_t *node_get_scan(const node_t *node);

/// @brief get the location of a node inside its source file
///
/// @param node the node to get the location of
///
/// @return the location of @p node
NODISCARD CONSTFN
CT_SCAN_API where_t node_get_location(const node_t *node);

/// @brief create a new node in a given file at a given location
///
/// @param scan the scanner that this node is in
/// @param where the location of this node
///
/// @return the created node
NODISCARD CONSTFN
CT_SCAN_API node_t *node_new(const scan_t *scan, where_t where);

/// @brief get the builtin node
/// node used for drivers that declare builtin symbols
///
/// @return the builtin node
NODISCARD CONSTFN
CT_SCAN_API const node_t *node_builtin(void);

/// @brief check if a node is the builtin node
///
/// @param node the node to check
///
/// @return whether or not @p node is the builtin node
NODISCARD CONSTFN
CT_SCAN_API bool node_is_builtin(const node_t *node);

/// @}

END_API
