#pragma once

#include <ctu_memory_api.h>

#include "core/analyze.h"

CT_BEGIN_API

typedef struct arena_t arena_t;

/// @defgroup global_memory Global memory allocation
/// @ingroup runtime
/// @brief Default global memory allocator
/// this is an intermediate layer to help with transitioning to pure arena allocation
/// its recommended to avoid this for new code and to remove it from existing code
/// @{

/// @brief get the global memory arena
///
/// @return the global memory arena
CT_MEMORY_API arena_t *get_global_arena(void);

/// @brief initialize the global memory arena
/// @warning this should be called with care, as it will overwrite the current arena
///
/// @param arena the arena to initialize
CT_MEMORY_API void init_global_arena(arena_t *arena);

/// @brief initialize gmp with a custom allocator
///
/// @param arena the allocator to use
CT_MEMORY_API void init_gmp_arena(IN_NOTNULL arena_t *arena);

/// @} // global_memory

CT_END_API
