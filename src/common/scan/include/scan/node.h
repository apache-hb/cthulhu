#pragma once

#include <ctu_scan_api.h>

#include "core/analyze.h"
#include "core/where.h"

#include "scan/scan.h"

#include <stdbool.h>

CT_BEGIN_API

/// @defgroup location Source location tracking
/// @brief AST source position tracking
/// @ingroup common
/// @{

/// @brief a position in a source file
typedef struct node_t
{
    /// @brief the scanner that this node is in
    const scan_t *scan;

    /// @brief the location of this node
    where_t where;
} node_t;

/// @brief nowhere in a source file
CT_SCAN_API extern const where_t kNowhere;

/// @brief get the associated source file of a node
///
/// @param node the node to get the source file of
///
/// @return the source file of @p node
CT_NODISCARD CT_PUREFN
CT_SCAN_API const scan_t *node_get_scan(IN_NOTNULL const node_t *node);

/// @brief get the location of a node inside its source file
///
/// @param node the node to get the location of
///
/// @return the location of @p node
CT_NODISCARD CT_PUREFN
CT_SCAN_API where_t node_get_location(IN_NOTNULL const node_t *node);

/// @brief create a new node in a given file at a given location
///
/// @param scan the scanner that this node is in
/// @param where the location of this node
///
/// @return the created node
CT_NODISCARD
CT_SCAN_API node_t *node_new(const scan_t *scan, where_t where);

/// @brief get the builtin node
/// node used for drivers that declare builtin symbols
///
/// @param name the name of the builtin file
/// @param arena the arena to allocate from
///
/// @return the builtin node
CT_NODISCARD CT_PUREFN
CT_SCAN_API node_t *node_builtin(IN_STRING const char *name, IN_NOTNULL arena_t *arena);

/// @brief check if a node is the builtin node
///
/// @param node the node to check
///
/// @return whether or not @p node is the builtin node
CT_NODISCARD CT_PUREFN
CT_SCAN_API bool node_is_builtin(IN_NOTNULL const node_t *node);

/// @}

CT_END_API
