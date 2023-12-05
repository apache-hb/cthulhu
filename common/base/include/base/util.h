#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdlib.h>

BEGIN_API

/// @brief hash a pointer value
///
/// @param ptr the pointer to hash
///
/// @return the hash of the pointer
NODISCARD CONSTFN
size_t ptrhash(const void *ptr);

END_API
