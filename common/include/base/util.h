#pragma once

#include <stdlib.h>

#include "analyze.h"

BEGIN_API

/**
 * @defgroup Memory General case memory managment
 * @{
 */

/**
 * @brief free a pointer
 *
 * @param ptr a pointer to valid memory allocated from @ref ctu_malloc
 */
void ctu_free(void *ptr);

/**
 * @brief allocate memory
 *
 * all memory allocated must be freed with @ref ctu_free.
 * will abort if memory cannot be allocated.
 *
 * @param size the size of the memory to allocate
 *
 * @return the allocated memory
 */
NODISCARD
void *ctu_malloc(size_t size);

/**
 * @brief reallocate memory
 *
 * memory must be originally allocated with @ref ctu_malloc, @ref ctu_realloc,
 * @ref ctu_strdup, @ref ctu_strndup, or @ref ctu_memdup.
 * all memory allocated must be freed with @ref ctu_free.
 * will abort if memory cannot be allocated.
 *
 * @param ptr the pointer to reallocate
 * @param size the new size the pointers memory should be
 *
 * @return the reallocated pointer
 */
NODISCARD
void *ctu_realloc(void *ptr, size_t size);

/**
 * @brief allocate a copy of a string
 *
 * all memory allocated must be freed with @ref ctu_free.
 * will abort if memory cannot be allocated.
 *
 * @param str the string to copy
 *
 * @return the allocated copy of the string
 */
NODISCARD
char *ctu_strdup(const char *str);

/**
 * @brief allocate a copy of a string with a maximum length
 *
 * all memory allocated must be freed with @ref ctu_free.
 * will abort if memory cannot be allocated.
 *
 * @param str the string to copy
 * @param len the maximum length of the string to copy
 *
 * @return the allocated copy of the string
 */
NODISCARD
char *ctu_strndup(const char *str, size_t len);

/**
 * @brief duplicate a memory region
 *
 * duplicate a region of memory and return a pointer to the new memory.
 * all memory allocated must be freed with @ref ctu_free.
 * will abort if memory cannot be allocated.
 *
 * @param ptr the pointer to duplicate
 * @param size the size of the memory to duplicate
 *
 * @return the duplicated memory
 */
NODISCARD
void *ctu_memdup(IN_READS(size) const void *ptr, size_t size);

/** @} */

NODISCARD
size_t ptrhash(const void *ptr);

/**
 * @brief init gmp with our own allocation functions
 */
void init_gmp(void);

#define BOX(name) ctu_memdup(&(name), sizeof(name)) ///< box a value onto the heap from the stack

END_API
