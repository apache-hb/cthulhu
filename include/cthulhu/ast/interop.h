#pragma once

#include "scan.h"

void flex_action(where_t *where, const char *text);
int flex_input(scan_t *scan, char *out, int size);
void flex_init(where_t *where);

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
