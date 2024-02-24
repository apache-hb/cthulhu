// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "arena/arena.h"
#include "scan/scan.h" // IWYU pragma: keep

/// @ingroup flex_bison_macros
/// @{

/// route memory for flex and bison though cthulhu allocators
#define FLEX_MEMORY(prefix)                                                 \
    inline void *prefix##alloc(size_t size, yyscan_t scanner)               \
    {                                                                       \
        scan_t *scan = yyget_extra(scanner);                                \
        arena_t *arena = scan_get_arena(scan);                              \
        return ARENA_MALLOC(size, "yyalloc", scan, arena);                  \
    }                                                                       \
    inline void *prefix##realloc(void *ptr, size_t bytes, yyscan_t scanner) \
    {                                                                       \
        arena_t *arena = scan_get_arena(yyget_extra(scanner));              \
        return arena_realloc(ptr, bytes, CT_ALLOC_SIZE_UNKNOWN, arena);        \
    }                                                                       \
    inline void prefix##free(void *ptr, yyscan_t scanner)                   \
    {                                                                       \
        arena_t *arena = scan_get_arena(yyget_extra(scanner));              \
        if (ptr == NULL)                                                    \
        {                                                                   \
            return;                                                         \
        }                                                                   \
        arena_free(ptr, CT_ALLOC_SIZE_UNKNOWN, arena);                         \
    }

/// @def FLEX_MEMORY(prefix)
/// @brief route memory for flex and bison though cthulhu allocators
/// @param prefix the prefix assigned to flex and bison functions

/// @}
