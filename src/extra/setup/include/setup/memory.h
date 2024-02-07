#pragma once

#include <ctu_setup_api.h>

#include "core/compiler.h"

typedef struct arena_t arena_t;

CT_BEGIN_API

/// @ingroup setup
/// @brief get the default allocator
///
/// @return the default allocator
CT_SETUP_API arena_t *ctu_default_alloc(void);

CT_END_API
