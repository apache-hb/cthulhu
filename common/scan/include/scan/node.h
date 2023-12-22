#pragma once

#include "core/analyze.h"

#include "scan/scan.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

BEGIN_API

/// @defgroup Location Source location tracking
/// @brief AST source position tracking
/// @ingroup Common
/// @{

typedef uint_fast64_t line_t;   ///< line number
typedef uint_fast64_t column_t; ///< column number

#define PRI_LINE PRIuFAST64
#define PRI_COLUMN PRIuFAST64

/// @brief a location inside a scanner
typedef struct
{
    line_t first_line; ///< the first line of the location
    line_t last_line;  ///< the last line of the location

    column_t first_column; ///< the first column of the location
    column_t last_column;  ///< the last column of the location
} where_t;

/// @brief a position in a source file
typedef struct node_t node_t;

void scan_init(void);

/// @brief get the associated source file of a node
///
/// @param node the node to get the source file of
///
/// @return the source file of @p node
NODISCARD CONSTFN
const scan_t *node_get_scan(const node_t *node);

/// @brief get the location of a node inside its source file
///
/// @param node the node to get the location of
///
/// @return the location of @p node
NODISCARD CONSTFN
where_t node_get_location(const node_t *node);

/// @brief create a new node in a given file at a given location
///
/// @param scan the scanner that this node is in
/// @param where the location of this node
///
/// @return the created node
NODISCARD CONSTFN
node_t *node_new(const scan_t *scan, where_t where);

/// @brief get the builtin node
/// node used for drivers that declare builtin symbols
///
/// @return the builtin node
NODISCARD CONSTFN
const node_t *node_builtin(void);

NODISCARD CONSTFN
bool node_is_builtin(const node_t *node);

/// @}

END_API
