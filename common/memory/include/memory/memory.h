#pragma once

#include "memory/arena.h"

BEGIN_API

/// @defgroup GlobalMemory Global memory allocation
/// @ingroup Memory
/// @brief Default global memory allocator
/// @{

/// @brief get the default allocator
///
/// @return the default allocator
arena_t *ctu_default_alloc(void);

/// @brief free a pointer allocated with ctu_malloc or ctu_realloc
/// @note @ref init_global_alloc must be consistent with the allocator used to allocate @p ptr
///
/// @param ptr the pointer to free, must not be NULL
void ctu_free(OUT_PTR_INVALID void *ptr);

/// @brief allocate a pointer from the default allocator
///
/// @param size the size of the allocation
///
/// @return the allocated pointer
NODISCARD CT_ALLOC(ctu_free)
RET_NOTNULL
void *ctu_malloc(IN_RANGE(!=, 0) size_t size);

NODISCARD CT_ALLOC(ctu_free) CT_ALLOC_SIZE(1)
RET_NOTNULL
void *ctu_malloc_info(IN_RANGE(!=, 0) size_t size, const char *name, const void *parent);

/// @brief reallocate a pointer from the default allocator
/// @note @ref init_global_alloc must be consistent with the allocator used to allocate @p ptr
///
/// @param ptr the pointer to reallocate
/// @param new_size the new size of the allocation
///
/// @return the reallocated pointer
NODISCARD CT_ALLOC(ctu_free) CT_ALLOC_SIZE(2)
RET_NOTNULL
void *ctu_realloc(IN_NOTNULL OUT_PTR_INVALID void *ptr, IN_RANGE(!=, 0) size_t new_size);

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
/// @param alloc the allocator to use
void init_gmp_alloc(IN_NOTNULL arena_t *alloc);

/// @brief initialize global allocator
/// @warning this function must be called before any other memory allocation function
///
/// @param alloc the allocator to use
void init_global_alloc(IN_NOTNULL arena_t *alloc);

/// @def BOX(name)
/// @brief box a value on the stack
/// @warning @p name must be an identifier, not an expression
#define BOX(name) ctu_memdup(&(name), sizeof(name))

/// @brief rename a memory allocation
///
/// @param ptr the pointer to rename
/// @param name the new name of the allocation
void ctu_mem_rename(IN_NOTNULL const void *ptr, IN_STRING const char *name);

/// @brief reparent a memory allocation
///
/// @param ptr the pointer to reparent
/// @param parent the new parent of the allocation
void ctu_mem_reparent(IN_NOTNULL const void *ptr, const void *parent);

/// @def MEM_RENAME(ptr, name)
/// @brief rename a pointer in the global allocator
/// @note this is a no-op if @ref CTU_TRACE_MEMORY is not defined
///
/// @param ptr the pointer to rename
/// @param name the new name of the pointer

/// @def MEM_REPARENT(ptr, parent)
/// @brief reparent a pointer in the global allocator
/// @note this is a no-op if @ref CTU_TRACE_MEMORY is not defined
///
/// @param ptr the pointer to reparent
/// @param parent the new parent of the pointer

/// @def MEM_ALLOC(size, name, parent)
/// @brief allocate memory from the global allocator
/// @note this is converted to @ref ctu_malloc if @ref CTU_TRACE_MEMORY is not defined
/// @pre @p size must be greater than 0
/// @warning this function must not be called before @ref init_global_alloc
///
/// @param size the size of the allocation, must be greater than 0
/// @param name the name of the allocation
/// @param parent the parent of the allocation
///
/// @return the allocated pointer

#if CTU_TRACE_MEMORY
#   define MEM_RENAME(ptr, name) ctu_mem_rename(ptr, name)
#   define MEM_REPARENT(ptr, parent) ctu_mem_reparent(ptr, parent)
#   define MEM_ALLOC(size, name, parent) ctu_malloc_info(size, name, parent)
#else
#   define MEM_RENAME(ptr, name)
#   define MEM_REPARENT(ptr, parent)
#   define MEM_ALLOC(size, name, parent) ctu_malloc(size)
#endif

/// @def MEM_IDENTIFY(ptr, name, parent)
/// @brief rename and reparent a pointer in the global allocator
/// @note this is a no-op if @ref CTU_TRACE_MEMORY is not defined
/// @warning identifying a pointer is not an atomic call and is implemented as
///          two separate calls to @ref MEM_RENAME and @ref MEM_REPARENT
///
/// @param ptr the pointer to rename
/// @param name the new name of the pointer
/// @param parent the new parent of the pointer
#define MEM_IDENTIFY(ptr, name, parent)                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        MEM_RENAME(ptr, name);                                                                                         \
        MEM_REPARENT(ptr, parent);                                                                                     \
    } while (0)

/// @} // GlobalMemory

END_API
