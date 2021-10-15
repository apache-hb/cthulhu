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
        scan_t *scan = yyget_extra(scanner); \
        logverbose("alloc [%p] [%p]", scan, scanner); \
        return arena_malloc(&scan->tokens, size); \
    } \
    inline void *realloc(void *ptr, size_t bytes, yyscan_t scanner) { \
        logverbose("realloc [%p]", scanner); \
        scan_t *scan = yyget_extra(scanner); \
        arena_realloc(&scan->tokens, &ptr, SIZE_MAX, bytes); \
        return ptr; \
    } \
    inline void free(void *ptr, yyscan_t scanner) { \
        UNUSED(ptr); \
        UNUSED(scanner); \
    }
