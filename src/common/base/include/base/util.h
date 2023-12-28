#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stddef.h>

BEGIN_API

/// @defgroup Util Utility functions
/// @brief Utility functions
/// @ingroup Base
/// @{

/// @brief hash a pointer value
///
/// @param ptr the pointer to hash
///
/// @return the hash of the pointer
NODISCARD CONSTFN
size_t ptrhash(const void *ptr);

/// @}

END_API
