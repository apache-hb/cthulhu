#pragma once

#include "cthulhu/ssa/ssa.h"

typedef struct reports_t reports_t;

/// @defgroup Check Tree form validation
/// @brief Validation for tree form IR
/// @ingroup Runtime
/// @{

/// @brief check the tree form IR
/// all found errors are reported to the reports object
///
/// @param reports the reports object
/// @param mods the modules to check
void check_tree(reports_t *reports, map_t *mods);

/// @}
