#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stddef.h>
#include <stdint.h>

BEGIN_API

/// @defgroup Memory
/// @brief Global and arena memory management
/// @{

#define ALLOC_SIZE_UNKNOWN SIZE_MAX
/// @def ALLOC_SIZE_UNKNOWN
/// @brief unknown allocation size constant
/// when freeing or reallocating memory, this can be used as the size
/// to indicate that the size is unknown. requires allocator support

typedef struct alloc_t alloc_t;

/// @brief malloc function pointer
///
/// @param self associated allocator
/// @param size the size of the allocation
/// @param name the optional name of the allocation
/// @param parent the optional parent of the allocation
///
/// @return the allocated pointer
/// @return NULL if the allocation failed
typedef void *(*malloc_t)(
        IN_NOTNULL alloc_t *self,
        IN_RANGE(!=, 0) size_t size,
        IN_STRING_OPT const char *name,
        const void *parent);

/// @brief realloc function pointer
///
/// @param self associated allocator
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
/// @param old_size the old size of the allocation
///
/// @return the reallocated pointer
/// @return NULL if the allocation failed
typedef void *(*realloc_t)(
        IN_NOTNULL alloc_t *self,
        OUT_PTR_INVALID void *ptr,
        IN_RANGE(!=, 0) size_t new_size,
        IN_RANGE(!=, 0) size_t old_size);

/// @brief free function pointer
///
/// @param self associated allocator
/// @param ptr the pointer to free
/// @param size the size of the allocation
typedef void (*free_t)(
        IN_NOTNULL alloc_t *self,
        OUT_PTR_INVALID void *ptr,
        IN_RANGE(!=, 0) size_t size);

/// @brief an allocator object
typedef struct alloc_t
{
    const char *name;        ///< the name of the allocator

    malloc_t arena_malloc;   ///< the malloc function
    realloc_t arena_realloc; ///< the realloc function
    free_t arena_free;       ///< the free function

    void *user; ///< user data
} alloc_t;

/// @brief release memory from a custom allocator
/// @note ensure the allocator is consistent with the allocator used to allocate @a ptr
///
/// @param alloc the allocator to use
/// @param ptr the pointer to free
/// @param size the size of the allocation
void arena_free(
    IN_NOTNULL alloc_t *alloc,
    OUT_PTR_INVALID void *ptr,
    IN_RANGE(!=, 0) size_t size);

/// @brief allocate memory from a custom allocator
///
/// @param alloc the allocator to use
/// @param size the size of the allocation, must be greater than 0
/// @param name the name of the allocation
///
/// @return the allocated pointer
NODISCARD CT_ALLOC(arena_free, 2)
RET_NOTNULL
void *arena_malloc(
    IN_NOTNULL alloc_t *alloc,
    IN_RANGE(!=, 0) size_t size,
    IN_STRING_OPT const char *name,
    const void *parent);

/// @brief resize a memory allocation from a custom allocator
/// @note ensure the allocator is consistent with the allocator used to allocate @a ptr
///
/// @param alloc the allocator to use
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
/// @param old_size the old size of the allocation
///
/// @return the reallocated pointer
NODISCARD
RET_NOTNULL
void *arena_realloc(
    IN_NOTNULL alloc_t *alloc,
    OUT_PTR_INVALID void *ptr,
    IN_RANGE(!=, 0) size_t new_size,
    IN_RANGE(!=, 0) size_t old_size);

/// @} // Memory Management

END_API
