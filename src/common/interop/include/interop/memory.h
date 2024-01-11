#pragma once

#include "memory/arena.h"
#include "scan/scan.h"

/// route memory for flex and bison though cthulhu allocators
#define FLEX_MEMORY(prefix)                                                 \
    inline void *prefix##alloc(size_t size, yyscan_t scanner)               \
    {                                                                       \
        scan_t *scan = yyget_extra(scanner);                                \
        arena_t *arena = scan_get_arena(scan);                              \
        return ARENA_MALLOC(arena, size, "yyalloc", scan);                  \
    }                                                                       \
    inline void *prefix##realloc(void *ptr, size_t bytes, yyscan_t scanner) \
    {                                                                       \
        arena_t *arena = scan_get_arena(yyget_extra(scanner));              \
        return arena_realloc(ptr, bytes, ALLOC_SIZE_UNKNOWN, arena);        \
    }                                                                       \
    inline void prefix##free(void *ptr, yyscan_t scanner)                   \
    {                                                                       \
        arena_t *arena = scan_get_arena(yyget_extra(scanner));              \
        if (ptr == NULL)                                                    \
        {                                                                   \
            return;                                                         \
        }                                                                   \
        arena_free(ptr, ALLOC_SIZE_UNKNOWN, arena);                         \
    }