#pragma once

#include "memory/arena.h"

BEGIN_API

/// @defgroup GlobalMemory Global memory allocation
/// @ingroup Memory
/// @brief Default global memory allocator
/// @{

alloc_t *ctu_default_alloc(void);

/// @brief free a pointer allocated with ctu_malloc or ctu_realloc
/// @note @a gDefaultAlloc must be consistent with the allocator used to allocate @a ptr
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

NODISCARD CT_ALLOC(ctu_free)
RET_NOTNULL
void *ctu_malloc_info(
    IN_RANGE(!=, 0) size_t size,
    const char *name,
    const void *parent);

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
void init_gmp_alloc(IN_NOTNULL alloc_t *alloc);

/// @brief initialize global allocator
/// @warning this function must be called before any other memory allocation function
///
/// @param alloc the allocator to use
void init_global_alloc(IN_NOTNULL alloc_t *alloc);

alloc_t *get_global_alloc(void);

#define BOX(name) ctu_memdup(&(name), sizeof(name))
/// @def BOX(name)
/// @brief box a value on the stack

void ctu_mem_rename(
    IN_NOTNULL const void *ptr,
    IN_STRING const char *name);

void ctu_mem_reparent(
    IN_NOTNULL const void *ptr,
    const void *parent);

void ctu_mem_identify(
    IN_NOTNULL const void *ptr,
    IN_STRING const char *name,
    const void *parent);

#if CTU_TRACE_MEMORY
#   define MEM_RENAME(ptr, name) ctu_mem_rename(ptr, name)
#   define MEM_REPARENT(ptr, parent) ctu_mem_reparent(ptr, parent)
#   define MEM_IDENTIFY(ptr, name, parent) ctu_mem_identify(ptr, name, parent)
#   define MEM_ALLOC(size, name, parent) ctu_malloc_info(size, name, parent)
#else
#   define MEM_RENAME(ptr, name)
#   define MEM_REPARENT(ptr, parent)
#   define MEM_IDENTIFY(ptr, name, parent)
#   define MEM_ALLOC(size, name, parent) ctu_malloc(size)
#endif

/// @} // GlobalMemory

END_API
