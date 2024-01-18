#pragma once

#include <ctu_scan_api.h>

#include "core/analyze.h"
#include "core/where.h"

#include "scan/scan.h"

#include <stdbool.h>

BEGIN_API

/// @defgroup location Source location tracking
/// @brief AST source position tracking
/// @ingroup common
/// @{

/// @brief a position in a source file
typedef struct node_t node_t;

/// @brief get the associated source file of a node
///
/// @param node the node to get the source file of
///
/// @return the source file of @p node
NODISCARD PUREFN
CT_SCAN_API const scan_t *node_get_scan(const node_t *node);

/// @brief get the location of a node inside its source file
///
/// @param node the node to get the location of
///
/// @return the location of @p node
NODISCARD PUREFN
CT_SCAN_API where_t node_get_location(const node_t *node);

/// @brief create a new node in a given file at a given location
///
/// @param scan the scanner that this node is in
/// @param where the location of this node
///
/// @return the created node
NODISCARD
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
