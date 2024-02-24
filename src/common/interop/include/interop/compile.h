// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_interop_api.h>

#include "scan/scan.h"

#include "base/panic.h"

#include <limits.h>

typedef struct io_t io_t;
typedef struct scan_t scan_t;

CT_BEGIN_API

/// @defgroup interop Flex/Bison interop utils
/// @brief Flex/Bison interop utils for easier integration
/// @ingroup common
/// @{

/// @brief scanner function callbacks for flex and bison
/// @note this should be created via the @a CT_CALLBACKS macro
typedef struct scan_callbacks_t
{
    int (*init)(scan_t *extra, void *scanner);                   ///< yylex_init_extra
    int (*parse)(void *scanner, scan_t *extra);                  ///< yyparse
    void *(*scan)(const char *text, size_t size, void *scanner); ///< yy_scan_bytes
    void (*destroy_buffer)(void *buffer, void *scanner);         ///< yy_delete_buffer
    void (*destroy)(void *scanner);                              ///< yylex_destroy
} scan_callbacks_t;

/// @def CT_CALLBACKS(id, prefix)
/// @brief callback boilerplate macro
///
/// generate callback boilerplate for @a scan_buffer
///
/// @param id the name of the generated callback object
/// @param prefix the prefix assigned to flex and bison functions

#define CT_CALLBACKS(id, prefix)                                                                \
    static int prefix##_##id##_##init(scan_t *extra, void *scanner)                             \
    {                                                                                           \
        return prefix##lex_init_extra(extra, (yyscan_t *)scanner);                              \
    }                                                                                           \
    static int prefix##_##id##_parse(void *scanner, scan_t *extra)                              \
    {                                                                                           \
        return prefix##parse(scanner, (scan_t *)extra);                                         \
    }                                                                                           \
    static void *prefix##_##id##_scan(const char *text, size_t size, void *scanner)             \
    {                                                                                           \
        CTASSERTF(size <= INT_MAX, #prefix "_scan (size = %zu > %d, name = %s)", size, INT_MAX, \
                  scan_path((scan_t *)scanner));                                                \
        return prefix##_scan_bytes(text, (int)size, scanner);                                   \
    }                                                                                           \
    static void prefix##_##id##_destroy_buffer(void *buffer, void *scanner)                     \
    {                                                                                           \
        prefix##_delete_buffer((YY_BUFFER_STATE)buffer, scanner);                               \
    }                                                                                           \
    static void prefix##_##id##_destroy(void *scanner)                                          \
    {                                                                                           \
        prefix##lex_destroy(scanner);                                                           \
    }                                                                                           \
    static const scan_callbacks_t id = {                                                        \
        .init = prefix##_##id##_##init,                                                         \
        .parse = prefix##_##id##_parse,                                                         \
        .scan = prefix##_##id##_scan,                                                           \
        .destroy_buffer = prefix##_##id##_destroy_buffer,                                       \
        .destroy = prefix##_##id##_destroy,                                                     \
    }

typedef enum parse_error_t
{
    /// @brief parse was successful
    eParseOk,

    /// @brief error initializing the scanner internals (our fault)
    eParseInitError,

    /// @brief entered invalid state during scanning (our fault)
    eParseScanError,

    /// @brief failed due to invalid input (users fault)
    eParseReject,

    eParseCount
} parse_error_t;

typedef struct parse_result_t
{
    parse_error_t result;

    union {
        void *tree;
        int error;
    };
} parse_result_t;

/// @brief parse the contents of a scanner into a language specific ast
/// @note callbacks should be generated via @a CT_CALLBACKS
///
/// @param extra the sanner being used
/// @param callbacks the flex/bison callbacks
///
/// @return the parse result
CT_INTEROP_API parse_result_t scan_buffer(IN_NOTNULL scan_t *extra,
                                          IN_NOTNULL const scan_callbacks_t *callbacks);

/// @}

CT_END_API
