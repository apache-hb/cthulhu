#pragma once

#include <stdlib.h>

#include "analyze.h"

BEGIN_API

/**
 * @defgroup Memory General case memory managment
 * @{
 */

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
char *ctu_strndup(IN_READS(len) const char *str, size_t len);

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

/**
 * @brief hash a pointer value
 * 
 * @param ptr the pointer to hash
 * @return the hash of the pointer
 */
NODISCARD CONSTFN 
size_t ptrhash(const void *ptr);

#define BOX(name) ctu_memdup(&(name), sizeof(name)) ///< box a value onto the heap from the stack

END_API
