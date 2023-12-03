#pragma once

#include "core/analyze.h"

#include "scan/scan.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup Location
 * @brief AST source position tracking
 * @{
 */

typedef uint_fast64_t line_t;   ///< line number
typedef uint_fast64_t column_t; ///< column number

#define PRI_LINE PRIuFAST64
#define PRI_COLUMN PRIuFAST64

/**
 * @brief a location inside a scanner
 */
typedef struct
{
    line_t firstLine; ///< the first line of the location
    line_t lastLine;  ///< the last line of the location

    column_t firstColumn; ///< the first column of the location
    column_t lastColumn;  ///< the last column of the location
} where_t;

/**
 * a position in a source file
 */
typedef struct node_t node_t;

/// @brief get the associated source file of a node
///
/// @param node the node to get the source file of
///
/// @return the source file of @a node
NODISCARD CONSTFN
scan_t *get_node_scanner(const node_t *node);

/// @brief get the location of a node inside its source file
///
/// @param node the node to get the location of
///
/// @return the location of @a node
NODISCARD CONSTFN
where_t get_node_location(const node_t *node);

/**
 * @brief create a new node in a given file at a given location
 *
 * @param scan the scanner that this node is in
 * @param where the location of this node
 * @return the created node
 */
NODISCARD CONSTFN
node_t *node_new(scan_t *scan, where_t where);

/// @brief get the builtin node
/// this node is used for drivers that declare builtin symbols
///
/// @return the builtin node
NODISCARD CONSTFN
node_t *node_builtin(void);

/// @brief get the invalid node
/// this node is used for positions that cannot be represented by a node
///
/// @return the invalid node
NODISCARD CONSTFN
node_t *node_invalid(void);

/// @brief check if a node can have its location queried
///
/// @param node the node to check
///
/// @return if @a node can have its location queried
NODISCARD CONSTFN
bool node_is_valid(const node_t *node);

/** @} */
