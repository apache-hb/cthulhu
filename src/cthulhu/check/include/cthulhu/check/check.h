#pragma once

#include <ctu_check_api.h>

#include "cthulhu/ssa/ssa.h"

typedef struct logger_t logger_t;

/// @defgroup check Tree form validation
/// @brief Validation for tree form IR
/// @ingroup runtime
/// @{

BEGIN_API

/// @brief check the tree form IR
/// all found errors are reported to the reports object
///
/// @param reports the reports object
/// @param mods the modules to check
CT_CHECK_API void check_tree(IN_NOTNULL logger_t *reports, IN_NOTNULL map_t *mods);

END_API

/// @}
