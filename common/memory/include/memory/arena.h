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

typedef struct arena_t arena_t;

typedef struct mem_t mem_t;

/// @brief malloc function pointer
///
/// @param event associated event
///
/// @return the allocated pointer
/// @return NULL if the allocation failed
typedef void *(*mem_alloc_t)(const mem_t *header, size_t size);

/// @brief realloc function pointer
///
/// @param event associated event
///
/// @return the reallocated pointer
/// @return NULL if the allocation failed
typedef void *(*mem_resize_t)(const mem_t *header, void *ptr, size_t new_size, size_t old_size);

/// @brief free function pointer
///
/// @param event associated event
typedef void (*mem_release_t)(const mem_t *header, void *ptr, size_t size);

typedef void (*mem_rename_t)(const mem_t *header, const void *ptr, const char *name);
typedef void (*mem_reparent_t)(const mem_t *header, const void *ptr, const void *parent);

/// @brief an allocator object
typedef struct arena_t
{
    const char *name;        ///< the name of the allocator

    mem_alloc_t      fn_malloc;   ///< the malloc function
    mem_resize_t     fn_realloc; ///< the realloc function
    mem_release_t    fn_free;       ///< the free function

    mem_rename_t     fn_rename;
    mem_reparent_t   fn_reparent;

    void *user; ///< user data
} arena_t;

arena_t *mem_arena(IN_NOTNULL const mem_t *event);

/// @brief release memory from a custom allocator
/// @note ensure the allocator is consistent with the allocator used to allocate @a ptr
///
/// @param alloc the allocator to use
/// @param ptr the pointer to free
/// @param size the size of the allocation
void arena_free(
    IN_NOTNULL arena_t *alloc,
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
    IN_NOTNULL arena_t *alloc,
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
    IN_NOTNULL arena_t *alloc,
    OUT_PTR_INVALID void *ptr,
    IN_RANGE(!=, 0) size_t new_size,
    IN_RANGE(!=, 0) size_t old_size);

void arena_rename(IN_NOTNULL arena_t *alloc, IN_NOTNULL const void *ptr, IN_STRING const char *name);
void arena_reparent(IN_NOTNULL arena_t *alloc, IN_NOTNULL const void *ptr, const void *parent);

#if CTU_TRACE_MEMORY
#   define ARENA_RENAME(alloc, ptr, name) arena_rename(alloc, ptr, name)
#   define ARENA_REPARENT(alloc, ptr, parent) arena_reparent(alloc, ptr, parent)
#   define ARENA_MALLOC(alloc, size, name, parent) arena_malloc(alloc, size, name, parent)
#else
#   define ARENA_RENAME(alloc, ptr, name)
#   define ARENA_REPARENT(alloc, ptr, parent)
#   define ARENA_MALLOC(alloc, size, name, parent) arena_malloc(alloc, size, NULL, NULL)
#endif

#define ARENA_IDENTIFY(alloc, ptr, name, parent) \
    do { \
        ARENA_RENAME(alloc, ptr, name); \
        ARENA_REPARENT(alloc, ptr, parent); \
    } while (0)

/// @} // Memory Management

END_API
