#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BEGIN_API

/// @defgroup Memory Arena memory allocation
/// @brief Global and arena memory management
/// @{

#ifdef WITH_DOXYGEN
#   define CTU_TRACE_MEMORY 1
#endif

/// @def CTU_TRACE_MEMORY
/// @brief a compile time flag to enable memory tracing
/// @note this is enabled by default in debug builds, see [The build guide](@ref building) for more information

/// @def ALLOC_SIZE_UNKNOWN
/// @brief unknown allocation size constant
/// when freeing or reallocating memory, this can be used as the size
/// to indicate that the size is unknown. requires allocator support
#define ALLOC_SIZE_UNKNOWN SIZE_MAX

typedef struct arena_t arena_t;
typedef struct mem_t mem_t;

/// @brief arena malloc callback
/// @pre @p size must be greater than 0.
///
/// @param header associated event
/// @param size the size of the allocation
///
/// @return the allocated pointer
/// @retval NULL if the allocation failed
typedef void *(*mem_alloc_t)(const mem_t *header, size_t size);

/// @brief arena realloc callback
/// @pre @p old_size must be either @ref ALLOC_SIZE_UNKNOWN or the size of the allocation.
/// @pre @p new_size must be greater than 0.
/// @pre @p ptr must not be NULL.
///
/// @param header associated event
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
/// @param old_size the old size of the allocation
///
/// @return the reallocated pointer
/// @retval NULL if the allocation failed
typedef void *(*mem_resize_t)(const mem_t *header, void *ptr, size_t new_size, size_t old_size);

/// @brief arena free callback
/// @pre @p size must be either @ref ALLOC_SIZE_UNKNOWN or the size of the allocation.
///
/// @param header associated event
/// @param ptr the pointer to free
/// @param size the size of the allocation.
typedef void (*mem_release_t)(const mem_t *header, void *ptr, size_t size);

/// @brief arena rename callback
///
/// @param header associated event
/// @param ptr the pointer to rename
/// @param name the new name of the pointer
typedef void (*mem_rename_t)(const mem_t *header, const void *ptr, const char *name);

/// @brief arena reparent callback
///
/// @param header associated event
/// @param ptr the pointer to reparent
/// @param parent the new parent of the pointer
typedef void (*mem_reparent_t)(const mem_t *header, const void *ptr, const void *parent);

/// @brief an allocator object
typedef struct arena_t
{
    /// @brief the name of the allocator
    const char *name;

    /// @brief the malloc function
    mem_alloc_t fn_malloc;

    /// @brief the realloc function
    mem_resize_t fn_realloc;

    /// @brief the free function
    mem_release_t fn_free;

    /// @brief the rename function
    /// @note this feature is optional
    mem_rename_t fn_rename;

    /// @brief the reparent function
    /// @note this feature is optional
    mem_reparent_t fn_reparent;

    /// @brief the user data
    void *user;
} arena_t;

/// @brief acquire the arena of an event header
/// @pre @p event must not be NULL.
///
/// @param event the event header
///
/// @return the arena of the event
arena_t *mem_arena(IN_NOTNULL const mem_t *event);

/// @brief release memory from a custom allocator
/// @note ensure the allocator is consistent with the allocator used to allocate @a ptr
///
/// @param alloc the allocator to use
/// @param ptr the pointer to free
/// @param size the size of the allocation
void arena_free(IN_NOTNULL arena_t *alloc, OUT_PTR_INVALID void *ptr, IN_RANGE(!=, 0) size_t size);

/// @brief allocate memory from a custom allocator
///
/// @param alloc the allocator to use
/// @param size the size of the allocation, must be greater than 0
/// @param name the name of the allocation
/// @param parent the parent of the allocation
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

/// @brief rename a pointer in a custom allocator
///
/// @param alloc the allocator to use
/// @param ptr the pointer to rename
/// @param name the new name of the pointer
void arena_rename(IN_NOTNULL arena_t *alloc, IN_NOTNULL const void *ptr, IN_STRING const char *name);

/// @brief reparent a pointer in a custom allocator
///
/// @param alloc the allocator to use
/// @param ptr the pointer to reparent
/// @param parent the new parent of the pointer
void arena_reparent(IN_NOTNULL arena_t *alloc, IN_NOTNULL const void *ptr, const void *parent);

/// @def ARENA_RENAME(alloc, ptr, name)
/// @brief rename a pointer in a custom allocator
/// @note this is a no-op if @ref CTU_TRACE_MEMORY is not defined
///
/// @param alloc the allocator to use
/// @param ptr the pointer to rename
/// @param name the new name of the pointer

/// @def ARENA_REPARENT(alloc, ptr, parent)
/// @brief reparent a pointer in a custom allocator
/// @note this is a no-op if @ref CTU_TRACE_MEMORY is not defined
///
/// @param alloc the allocator to use
/// @param ptr the pointer to reparent
/// @param parent the new parent of the pointer

/// @def ARENA_MALLOC(alloc, size, name, parent)
/// @brief allocate memory from a custom allocator
/// @note this is converted to @ref arena_malloc if @ref CTU_TRACE_MEMORY is not defined
///
/// @param alloc the allocator to use
/// @param size the size of the allocation, must be greater than 0
/// @param name the name of the allocation
/// @param parent the parent of the allocation
///
/// @return the allocated pointer

#if CTU_TRACE_MEMORY
#   define ARENA_RENAME(alloc, ptr, name) arena_rename(alloc, ptr, name)
#   define ARENA_REPARENT(alloc, ptr, parent) arena_reparent(alloc, ptr, parent)
#   define ARENA_MALLOC(alloc, size, name, parent) arena_malloc(alloc, size, name, parent)
#else
#   define ARENA_RENAME(alloc, ptr, name)
#   define ARENA_REPARENT(alloc, ptr, parent)
#   define ARENA_MALLOC(alloc, size, name, parent) arena_malloc(alloc, size, NULL, NULL)
#endif

/// @def ARENA_IDENTIFY(alloc, ptr, name, parent)
/// @brief rename and reparent a pointer in a custom allocator
/// @note this is a no-op if @ref CTU_TRACE_MEMORY is not defined
/// @warning identifying a pointer is not an atomic call and is implemented as
///          two separate calls to @ref ARENA_RENAME and @ref ARENA_REPARENT
///
/// @param alloc the allocator to use
/// @param ptr the pointer to rename

#define ARENA_IDENTIFY(alloc, ptr, name, parent)                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        ARENA_RENAME(alloc, ptr, name);                                                                                \
        ARENA_REPARENT(alloc, ptr, parent);                                                                            \
    } while (0)

/// @} // Memory Management

END_API
