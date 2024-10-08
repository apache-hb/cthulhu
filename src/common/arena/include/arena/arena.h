// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_arena_api.h>
#include <ctu_config.h>

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

CT_BEGIN_API

/// @defgroup memory Arena memory allocation
/// @brief Global and arena memory management
/// the arena_* functions provide strong garuntees for memory allocation and deallocation
/// with pre and post checks to catch possible incorrect usage.
/// for weaker garuntees, the @ref memory_opt functions can be used.
/// @ingroup common
/// @{

/// @def CTU_TRACE_MEMORY
/// @brief a compile time flag to enable memory tracing
/// @note this is enabled by default in debug builds, see [The build guide](@ref building) for more information

#ifdef WITH_DOXYGEN
#   define CTU_TRACE_MEMORY 0
#endif

/// @def CT_ALLOC_SIZE_UNKNOWN
/// @brief unknown allocation size constant
/// when freeing or reallocating memory, this can be used as the size
/// to indicate that the size is unknown. requires allocator to support this.
#define CT_ALLOC_SIZE_UNKNOWN SIZE_MAX

/// @brief a memory allocator
typedef struct arena_t arena_t;

/// @brief arena malloc callback
/// @pre @p size must be greater than 0.
///
/// @param size the size of the allocation
/// @param user user data
///
/// @return the allocated pointer
/// @retval NULL if the allocation failed
typedef void *(*mem_alloc_t)(size_t size, void *user);

/// @brief arena realloc callback
///
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
/// @param old_size the old size of the allocation
/// @param user user data
///
/// @return the reallocated pointer
/// @retval NULL if the allocation failed
typedef void *(*mem_resize_t)(void *ptr, size_t new_size, size_t old_size, void *user);

/// @brief arena free callback
///
/// @param ptr the pointer to free
/// @param size the size of the allocation.
/// @param user user data
typedef void (*mem_release_t)(void *ptr, size_t size, void *user);

/// @brief arena rename callback
///
/// @param ptr the pointer to rename
/// @param name the new name of the pointer
/// @param user user data
typedef void (*mem_rename_t)(const void *ptr, const char *name, void *user);

/// @brief arena reparent callback
///
/// @param ptr the pointer to reparent
/// @param parent the new parent of the pointer
/// @param user user data
typedef void (*mem_reparent_t)(const void *ptr, const void *parent, void *user);

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
    /// @note this function is optional
    mem_rename_t fn_rename;

    /// @brief the reparent function
    /// @note this function is optional
    mem_reparent_t fn_reparent;

    /// @brief user data pointer
    void *user;
} arena_t;

/// @brief release memory from a custom allocator
/// @pre @p ptr must be allocated from @p arena
/// @pre @p size must be > 0
/// @pre @p arena must not be NULL
///
/// @param arena the allocator to use
/// @param ptr the pointer to free
/// @param size the size of the allocation
CT_ARENA_API void arena_free(
    STA_RELEASE void *ptr,
    IN_DOMAIN(>, 0) size_t size,
    IN_NOTNULL arena_t *arena);

/// @brief allocate memory from a custom allocator
/// @pre @p size must be > 0
/// @pre @p arena must not be NULL
///
/// @param arena the allocator to use
/// @param size the size of the allocation, must be greater than 0
///
/// @return the allocated pointer
CT_NODISCARD CT_ALLOC(arena_free) CT_ALLOC_SIZE(1)
RET_NOTNULL
CT_ARENA_API void *arena_malloc(
    IN_DOMAIN(>, 0) size_t size,
    IN_NOTNULL arena_t *arena);

/// @brief allocate memory from a custom allocator
/// @pre @p size must be > 0
/// @pre @p arena must not be NULL
/// @pre @p name must either be NULL or a valid null terminated string
///
/// @param arena the allocator to use
/// @param size the size of the allocation, must be greater than 0
/// @param name the name of the allocation
/// @param parent the parent of the allocation
///
/// @return the allocated pointer
CT_NODISCARD CT_ALLOC(arena_free) CT_ALLOC_SIZE(1)
RET_NOTNULL
CT_ARENA_API void *arena_malloc_info(
    IN_DOMAIN(>, 0) size_t size,
    const char *name,
    const void *parent,
    IN_NOTNULL arena_t *arena);

/// @brief resize a memory allocation from a custom allocator
/// @pre @p ptr must be allocated from @p arena
/// @pre @p new_size must be > 0
/// @pre @p old_size must be the size originally allocated
/// @pre @p arena must not be NULL
///
/// @param arena the allocator to use
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
/// @param old_size the old size of the allocation
///
/// @return the reallocated pointer
CT_NODISCARD CT_ALLOC(arena_free) CT_ALLOC_SIZE(2)
RET_NOTNULL
CT_ARENA_API void *arena_realloc(
    STA_RELEASE void *ptr,
    IN_DOMAIN(>, 0) size_t new_size,
    IN_DOMAIN(>, 0) size_t old_size,
    IN_NOTNULL arena_t *arena);

/// @brief allocate a copy of a string from a custom allocator
/// @pre @p str must be a valid, null terminated, string
/// @pre @p arena must not be NULL
///
/// @param str the string to copy
/// @param arena the allocator to use
///
/// @return the allocated copy of the string
CT_NODISCARD CT_ALLOC(arena_free)
RET_NOTNULL
CT_ARENA_API char *arena_strdup(
    IN_STRING const char *str,
    IN_NOTNULL arena_t *arena);

/// @brief allocate a copy of a string with a maximum length from a custom allocator
/// @pre @p str must be a valid string of at least @p len characters
/// @pre @p arena must not be NULL
///
/// @param str the string to copy
/// @param len the maximum length of the string to copy
/// @param arena the allocator to use
///
/// @return the allocated copy of the string
CT_NODISCARD CT_ALLOC(arena_free)
RET_NOTNULL
CT_ARENA_API char *arena_strndup(
    STA_READS(len) const char *str,
    size_t len,
    IN_NOTNULL arena_t *arena);

/// @brief duplicate a memory region from a custom allocator
/// duplicate a region of memory and return a pointer to the new memory.
/// @pre @p ptr must be a valid pointer to @p size bytes of memory
/// @pre @p size must be > 0
/// @pre @p arena must not be NULL
///
/// @param ptr the pointer to duplicate
/// @param size the size of the memory to duplicate
/// @param arena the allocator to use
///
/// @return the duplicated memory
CT_NODISCARD CT_ALLOC(arena_free) CT_ALLOC_SIZE(2)
RET_NOTNULL
CT_ARENA_API void *arena_memdup(
    STA_READS(size) const void *ptr,
    IN_DOMAIN(>, 0) size_t size,
    IN_NOTNULL arena_t *arena);

/// @defgroup memory_opt Failable arena allocation
/// @brief Failable arena allocation
/// these allocation functions are used when the allocation is not critical and can fail.
/// they also perform less validation on the input parameters than the @ref memory functions.
/// @ingroup memory
/// @{

/// @brief release memory from a custom allocator
/// @pre @p ptr must be allocated from @p arena
///
/// @param arena the allocator to use
/// @param ptr the pointer to free
/// @param size the size of the allocation
CT_ARENA_API void arena_opt_free(
    STA_RELEASE void *ptr,
    IN_DOMAIN(>, 0) size_t size,
    IN_NOTNULL arena_t *arena);

/// @brief allocate memory from a custom allocator
/// @pre @p arena must not be NULL
///
/// @param arena the allocator to use
/// @param size the size of the allocation, must be greater than 0
///
/// @return the allocated pointer
CT_NODISCARD CT_ALLOC(arena_opt_free) CT_ALLOC_SIZE(1)
CT_ARENA_API void *arena_opt_malloc(
    IN_DOMAIN(>, 0) size_t size,
    IN_NOTNULL arena_t *arena);

/// @brief allocate memory from a custom allocator
/// @pre @p ptr must be allocated from @p arena
/// @pre @p arena must not be NULL
///
/// @param arena the allocator to use
/// @param size the size of the allocation, must be greater than 0
/// @param name the name of the allocation
/// @param parent the parent of the allocation
///
/// @return the allocated pointer
CT_NODISCARD CT_ALLOC(arena_opt_free) CT_ALLOC_SIZE(1)
CT_ARENA_API void *arena_opt_malloc_info(
    IN_DOMAIN(>, 0) size_t size,
    const char *name,
    const void *parent,
    IN_NOTNULL arena_t *arena);

/// @brief resize a memory allocation from a custom allocator
/// @pre @p ptr must be allocated from @p arena
///
/// @param arena the allocator to use
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
/// @param old_size the old size of the allocation
///
/// @return the reallocated pointer
CT_NODISCARD CT_ALLOC(arena_opt_free)
CT_ARENA_API void *arena_opt_realloc(
    STA_RELEASE void *ptr,
    IN_DOMAIN(>, 0) size_t new_size,
    IN_DOMAIN(>, 0) size_t old_size,
    IN_NOTNULL arena_t *arena);

/// @brief allocate a copy of a string from a custom allocator
/// @pre @p str must be a valid, null terminated, string
/// @pre @p arena must not be NULL
///
/// @param str the string to copy
/// @param arena the allocator to use
///
/// @return the allocated copy of the string
CT_NODISCARD CT_ALLOC(arena_free)
CT_ARENA_API char *arena_opt_strdup(
    IN_STRING const char *str,
    IN_NOTNULL arena_t *arena);

/// @brief allocate a copy of a string with a maximum length from a custom allocator
/// @pre @p str must be a valid string of at least @p len characters
/// @pre @p arena must not be NULL
///
/// @param str the string to copy
/// @param len the maximum length of the string to copy
/// @param arena the allocator to use
///
/// @return the allocated copy of the string
CT_NODISCARD CT_ALLOC(arena_free)
CT_ARENA_API char *arena_opt_strndup(
    STA_READS(len) const char *str,
    IN_DOMAIN(>, 0) size_t len,
    IN_NOTNULL arena_t *arena);

/// @brief duplicate a memory region from a custom allocator
/// duplicate a region of memory and return a pointer to the new memory.
/// @pre @p ptr must be a valid pointer to @p size bytes of memory
/// @pre @p arena must not be NULL
///
/// @param ptr the pointer to duplicate
/// @param size the size of the memory to duplicate
/// @param arena the allocator to use
///
/// @return the duplicated memory
CT_NODISCARD CT_ALLOC(arena_free) CT_ALLOC_SIZE(2)
CT_ARENA_API void *arena_opt_memdup(
    STA_READS(size) const void *ptr,
    IN_DOMAIN(>, 0) size_t size,
    IN_NOTNULL arena_t *arena);

/// @}

/// @brief get the user data pointer from an arena
///
/// @param arena the allocator to use
///
/// @return the user data pointer
CT_NODISCARD
CT_ARENA_API void *arena_data(IN_NOTNULL arena_t *arena);

/// @brief rename a pointer in a custom allocator
///
/// @param arena the allocator to use
/// @param ptr the pointer to rename
/// @param name the new name of the pointer
CT_ARENA_API void arena_rename(IN_NOTNULL const void *ptr, IN_STRING const char *name, IN_NOTNULL arena_t *arena);

/// @brief reparent a pointer in a custom allocator
///
/// @param arena the allocator to use
/// @param ptr the pointer to reparent
/// @param parent the new parent of the pointer
CT_ARENA_API void arena_reparent(IN_NOTNULL const void *ptr, const void *parent, IN_NOTNULL arena_t *arena);

/// @def ARENA_RENAME(arena, ptr, name)
/// @brief rename a pointer in a custom allocator
/// @note this is a no-op if @a CTU_TRACE_MEMORY is not defined
///
/// @param ptr the pointer to rename
/// @param name the new name of the pointer
/// @param arena the allocator to use

/// @def ARENA_REPARENT(arena, ptr, parent)
/// @brief reparent a pointer in a custom allocator
/// @note this is a no-op if @a CTU_TRACE_MEMORY is not defined
///
/// @param ptr the pointer to reparent
/// @param parent the new parent of the pointer
/// @param arena the allocator to use

/// @def ARENA_MALLOC(arena, size, name, parent)
/// @brief allocate memory from a custom allocator
/// @note this is converted to @a arena_malloc if @a CTU_TRACE_MEMORY is not defined
///
/// @param size the size of the allocation, must be greater than 0
/// @param name the name of the allocation
/// @param parent the parent of the allocation
/// @param arena the allocator to use
///
/// @return the allocated pointer

// cast to (const void*) to support using these in C++
#if CTU_TRACE_MEMORY
#   define ARENA_RENAME(ptr, name, arena) arena_rename((const void*)(ptr), name, arena)
#   define ARENA_REPARENT(ptr, parent, arena) arena_reparent((const void*)(ptr), (const void*)(parent), arena)
#   define ARENA_MALLOC(size, name, parent, arena) arena_malloc_info(size, name, (const void*)(parent), arena)
#   define ARENA_OPT_MALLOC(size, name, parent, arena) arena_opt_malloc_info(size, name, (const void*)(parent), arena)
#else
#   define ARENA_RENAME(arena, ptr, name)
#   define ARENA_REPARENT(arena, ptr, parent)
#   define ARENA_MALLOC(size, name, parent, arena) arena_malloc(size, arena)
#   define ARENA_OPT_MALLOC(size, name, parent, arena) arena_opt_malloc(size, arena)
#endif

/// @def ARENA_IDENTIFY(arena, ptr, name, parent)
/// @brief rename and reparent a pointer in a custom allocator
/// @note this is a no-op if @a CTU_TRACE_MEMORY is not defined
/// @warning identifying a pointer is not an atomic call and is implemented as
///          two separate calls to @a ARENA_RENAME and @a ARENA_REPARENT
/// @warning @p name must be an expression that is evaluated in the argument list
/// @warning @p parent must be an address on the heap
///
/// @param ptr the pointer to rename
/// @param name the new name of the pointer
/// @param parent the new parent of the pointer
/// @param arena the allocator to use

#define ARENA_IDENTIFY(ptr, name, parent, arena)                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        ARENA_RENAME(ptr, name, arena);                                                                                \
        ARENA_REPARENT(ptr, parent, arena);                                                                            \
    } while (0)

/// @} // Memory Management

CT_END_API
