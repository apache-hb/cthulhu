#pragma once

#include "cthulhu/ssa/ssa.h"

typedef struct logger_t logger_t;

/// @defgroup check Tree form validation
/// @brief Validation for tree form IR
/// @ingroup runtime
/// @{

/// @brief check the tree form IR
/// all found errors are reported to the reports object
///
/// @param reports the reports object
/// @param mods the modules to check
void check_tree(logger_t *reports, map_t *mods);

/// @}
