#pragma once

#include "cthulhu/util/macros.h"
#include "scan.h"

/**
 * @defgroup LocationTracking AST location tracking
 * @brief AST source position tracking
 * @{
 */

/**
 * a position in a source file
 */
typedef struct {
    scan_t *scan;  ///< the source file
    where_t where; ///< the location of this node in the source file
} node_t;

/**
 * @brief create a new node in a given file at a given location
 *
 * @param scan the scanner that this node is in
 * @param where the location of this node
 * @return the created node
 */
node_t *node_new(scan_t *scan, where_t where);

/**
 * @brief a sentinel node that is used to mark a location as being generated by the compiler
 * @note this is a singleton and as such its safe to compare against it with pointer equality
 *
 * @return the builtin node
 */
const node_t *node_builtin(void);

/** @} */
