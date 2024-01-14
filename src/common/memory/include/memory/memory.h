#pragma once

#include "memory/arena.h"

BEGIN_API

/// @defgroup GlobalMemory Global memory allocation
/// @ingroup Memory
/// @brief Default global memory allocator
/// @{

/// @brief get the global memory arena
///
/// @return the global memory arena
arena_t *get_global_arena(void);

/// @brief initialize the global memory arena
/// @warning this should be called with care, as it will overwrite the current arena
///
/// @param arena the arena to initialize
void init_global_arena(arena_t *arena);

/// @brief initialize gmp with a custom allocator
///
/// @param arena the allocator to use
void init_gmp_arena(IN_NOTNULL arena_t *arena);

/// @} // GlobalMemory

END_API
