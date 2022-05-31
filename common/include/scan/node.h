#pragma once

#include "scan/scan.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup LocationTracking AST location tracking
 * @brief AST source position tracking
 * @{
 */

typedef uint_fast64_t line_t;   ///< line number
typedef uint_fast64_t column_t; ///< column number

/*
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
typedef unsigned node_t;

scan_t get_node_scanner(node_t node);
where_t get_node_location(node_t node);

/**
 * @brief create a new node in a given file at a given location
 *
 * @param scan the scanner that this node is in
 * @param where the location of this node
 * @return the created node
 */
node_t node_new(scan_t scan, where_t where);

node_t node_builtin(void);

node_t node_invalid(void);

bool node_is_valid(node_t node);

/** @} */
