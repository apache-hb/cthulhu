#pragma once

#include "memory/arena.h"

BEGIN_API

/// @defgroup GlobalMemory Global memory allocation
/// @ingroup Memory
/// @brief Default global memory allocator
/// @{

/// @brief the default allocator
/// @note this is the default allocator used by @a ctu_malloc, @a ctu_realloc, and @a ctu_free
/// @note exercise caution swapping this allocator during a compiler run
extern alloc_t gDefaultAlloc;

/// @brief free a pointer allocated with ctu_malloc or ctu_realloc
/// @note @a gDefaultAlloc must be consistent with the allocator used to allocate @a ptr
///
/// @param ptr the pointer to free, must not be NULL
void ctu_free(IN_NOTNULL OUT_PTR_INVALID void *ptr);

/// @brief allocate a pointer from the default allocator
///
/// @param size the size of the allocation
///
/// @return the allocated pointer
NODISCARD CT_ALLOC(ctu_free)
RET_NOTNULL
void *ctu_malloc(IN_RANGE(!=, 0) size_t size);

/// @brief reallocate a pointer from the default allocator
/// @note @a gDefaultAlloc must be consistent with the allocator used to allocate @a ptr
///
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
///
/// @return the reallocated pointer
NODISCARD CT_ALLOC(ctu_free)
RET_NOTNULL
void *ctu_realloc(
        IN_NOTNULL OUT_PTR_INVALID void *ptr,
        IN_RANGE(!=, 0) size_t new_size);

///
/// @brief allocate a copy of a string
///
/// @note all memory allocated must be freed with @ref ctu_free.
/// will abort if memory cannot be allocated.
///
/// @param str the string to copy
///
/// @return the allocated copy of the string
NODISCARD
char *ctu_strdup(IN_STRING const char *str);

/// @brief allocate a copy of a string with a maximum length
///
/// all memory allocated must be freed with @ref ctu_free.
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
/// all memory allocated must be freed with @ref ctu_free.
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
/// @param alloc the allocator to use
void init_gmp(IN_NOTNULL alloc_t *alloc);

#define BOX(name) ctu_memdup(&(name), sizeof(name))
/// @def BOX(name)
/// @brief box a value on the stack

/// @} // GlobalMemory

END_API
