#pragma once

#include "core/compiler.h"

typedef struct arena_t arena_t;

BEGIN_API

/// @ingroup defaults
/// @brief get the default allocator
///
/// @return the default allocator
arena_t *ctu_default_alloc(void);

END_API
