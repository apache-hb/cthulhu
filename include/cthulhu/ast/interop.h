#pragma once

#include "scan.h"

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

#define YY_USER_ACTION \
    flex_action(yylloc, yytext);

#define YY_INPUT(buffer, result, size)          \
    result = flex_input(yyextra, buffer, size); \
    if (result <= 0) { result = YY_NULL; }

#define YY_USER_INIT \
    flex_init(yylloc);

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

/**
 * @defgroup FlexBisonMacros Flex and Bison helper macros
 * @{
 * @def YY_USER_ACTION used to track source locations inside flex and bison
 * @def YY_INPUT(buffer, result, size) used to read input from flex and bison
 * @def YY_USER_INIT used to initialize flex and bison
 * @def FLEX_MEMORY(alloc, realloc, free) used to route memory for flex and bison though cthulhu allocators
 * @}
 */
