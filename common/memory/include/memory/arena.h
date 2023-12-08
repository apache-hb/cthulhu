#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

typedef struct mem_t mem_t;

typedef struct malloc_event_t
{
    // required params
    size_t size;

    /// @brief the name of the allocation
    const char *name;

    /// @brief the parent ptr of the allocation
    const void *parent;
} malloc_event_t;

typedef struct realloc_event_t
{
    // required params
    void *ptr;
    size_t new_size;

    /// @brief the old size of the allocation
    /// @note may be @ref ALLOC_SIZE_UNKNOWN
    size_t old_size;
} realloc_event_t;

typedef struct free_event_t
{
    // required params
    void *ptr;

    /// @brief the size of the allocation
    /// @note may be @ref ALLOC_SIZE_UNKNOWN
    size_t size;
} free_event_t;

/// @brief malloc function pointer
///
/// @param event associated event
///
/// @return the allocated pointer
/// @return NULL if the allocation failed
typedef void *(*fn_malloc_t)(const mem_t *header, malloc_event_t event);

/// @brief realloc function pointer
///
/// @param event associated event
///
/// @return the reallocated pointer
/// @return NULL if the allocation failed
typedef void *(*fn_realloc_t)(const mem_t *header, realloc_event_t event);

/// @brief free function pointer
///
/// @param event associated event
typedef void (*fn_free_t)(const mem_t *header, free_event_t event);

/// @brief an allocator object
typedef struct alloc_t
{
    const char *name;        ///< the name of the allocator

    fn_malloc_t arena_malloc;   ///< the malloc function
    fn_realloc_t arena_realloc; ///< the realloc function
    fn_free_t arena_free;       ///< the free function

    void *user; ///< user data
} alloc_t;

alloc_t *mem_arena(IN_NOTNULL const mem_t *event);

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
