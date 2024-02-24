// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_check_api.h>

#include "core/analyze.h"

typedef struct logger_t logger_t;
typedef struct tree_t tree_t;
typedef struct vector_t vector_t;
typedef struct arena_t arena_t;

/// @defgroup check Tree form validation
/// @brief Validation for tree form IR
/// @ingroup runtime
/// @{

CT_BEGIN_API

/// @brief check the tree form IR
/// all found errors are reported to the reports object
///
/// @param reports the reports object
/// @param mods the modules to check
/// @param arena the arena to allocate in
CT_CHECK_API void check_tree(IN_NOTNULL logger_t *reports, IN_NOTNULL vector_t *mods, IN_NOTNULL arena_t *arena);

CT_END_API

/// @}
