#pragma once

#include "memory/arena.h"

BEGIN_API

/// @defgroup GlobalMemory Global memory allocation
/// @ingroup Memory
/// @brief Default global memory allocator
/// @{

arena_t *get_global_arena(void);
void init_global_arena(arena_t *arena);

///
/// @brief allocate a copy of a string
///
/// will abort if memory cannot be allocated.
///
/// @param str the string to copy
///
/// @return the allocated copy of the string
NODISCARD
char *ctu_strdup(IN_STRING const char *str);

/// @brief allocate a copy of a string with a maximum length
///
/// will abort if memory cannot be allocated.
///
/// @param str the string to copy
/// @param len the maximum length of the string to copy
///
/// @return the allocated copy of the string
NODISCARD
char *ctu_strndup(IN_READS(len) const char *str, size_t len);

/// @brief duplicate a memory region
/// duplicate a region of memory and return a pointer to the new memory.
/// will abort if memory cannot be allocated.
///
/// @param ptr the pointer to duplicate
/// @param size the size of the memory to duplicate
///
/// @return the duplicated memory
NODISCARD
void *ctu_memdup(IN_READS(size) const void *ptr, IN_RANGE(>, 0) size_t size);

/// @brief initialize gmp with a custom allocator
///
/// @param arena the allocator to use
void init_gmp_arena(IN_NOTNULL arena_t *arena);

/// @} // GlobalMemory

END_API
