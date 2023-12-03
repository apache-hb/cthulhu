#pragma once

#include "core/analyze.h"

#include <stddef.h>
#include <stdint.h>

BEGIN_API

#define ALLOC_SIZE_UNKNOWN SIZE_MAX

/// @defgroup Memory
/// @brief Global and arena memory management
/// @{

typedef struct alloc_t alloc_t;

/// @brief malloc function pointer
///
/// @param self associated allocator
/// @param size the size of the allocation
/// @param name the name of the allocation
///
/// @return the allocated pointer
/// @return NULL if the allocation failed
typedef void *(*malloc_t)(
        IN_NOTNULL alloc_t *self,
        IN_RANGE(!=, 0) size_t size,
        IN_STRING_OPT const char *name);

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
} alloc_t;

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
NODISCARD ALLOC(ctu_free)
RET_NOTNULL
void *ctu_malloc(IN_RANGE(!=, 0) size_t size);

/// @brief reallocate a pointer from the default allocator
/// @note @a gDefaultAlloc must be consistent with the allocator used to allocate @a ptr
///
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
///
/// @return the reallocated pointer
NODISCARD ALLOC(ctu_free)
RET_NOTNULL
void *ctu_realloc(
        IN_NOTNULL OUT_PTR_INVALID void *ptr,
        IN_RANGE(!=, 0) size_t new_size);

/// @brief initialize gmp with a custom allocator
///
/// @param alloc the allocator to use
void init_gmp(IN_NOTNULL alloc_t *alloc);

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
NODISCARD ALLOC(arena_free, 2)
RET_NOTNULL
void *arena_malloc(
    IN_NOTNULL alloc_t *alloc,
    IN_RANGE(!=, 0) size_t size,
    IN_STRING_OPT const char *name);

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
