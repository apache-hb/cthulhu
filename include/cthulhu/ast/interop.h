#pragma once

#if !COMPILER_SOURCE
#   include "cthulhu/util/macros.h"
DISABLE_SAL
#endif

#include "scan.h"

/**
 * @defgroup FlexBisonMacros Flex and Bison helper macros
 * @brief Helper macros for flex and bison driver frontends
 * @{
 */

/**
 * @brief tracks the current source position
 * @see https://ftp.gnu.org/old-gnu/Manuals/flex-2.5.4/html_node/flex_14.html
 * 
 * @param where a pointer to the current location
 * @param text current source text
 */
void flex_action(where_t *where, const char *text) NONULL;

/**
 * @brief retrevies more input for flex
 * 
 * @param scan the source scanner
 * @param out output buffer to write to
 * @param size total number of characters to write
 * @return number of characters written
 */
int flex_input(scan_t *scan, char *out, int size) NONULL;

/**
 * @brief initialize source location tracking
 * 
 * @param where the source location to initialize
 */
void flex_init(where_t *where) NONULL;


/**
 * used to track source locations inside flex and bison
 */
#define YY_USER_ACTION \
    flex_action(yylloc, yytext);

/**
 * used to read input from flex and bison
 */
#define YY_INPUT(buffer, result, size)          \
    result = flex_input(yyextra, buffer, size); \
    if (result <= 0) { result = YY_NULL; }

/**
 * used to initialize flex and bison
 */
#define YY_USER_INIT \
    flex_init(yylloc);

/**
 * used to route memory for flex and bison though cthulhu allocators
 */
#define FLEX_MEMORY(alloc, realloc, free) \
    inline void *alloc(size_t size, yyscan_t scanner) { \
        UNUSED(scanner); \
        return ctu_malloc(size); \
    } \
    inline void *realloc(void *ptr, size_t bytes, yyscan_t scanner) { \
        UNUSED(scanner); \
        return ctu_realloc(ptr, bytes); \
    } \
    inline void free(void *ptr, yyscan_t scanner) { \
        UNUSED(scanner); \
        ctu_free(ptr); \
    }

/** @} */

