#pragma once

#include <ctu_interop_api.h>

#include "core/where.h"

#include "base/panic.h"

typedef struct scan_t scan_t;

CT_BEGIN_API

/// @defgroup flex_bison_macros Helpers for flex and bison driver frontends
/// @ingroup interop
/// @{

/// @brief tracks the current source position
/// @see https://ftp.gnu.org/old-gnu/Manuals/flex-2.5.4/html_node/flex_14.html
///
/// @param where a pointer to the current location
/// @param text current source text
CT_INTEROP_API void flex_action(IN_NOTNULL where_t *where, IN_STRING const char *text);

/// @brief retrevies more input for flex
///
/// @param scan the source scanner
/// @param out output buffer to write to
/// @param size total number of characters to write
///
/// @return number of characters written
CT_INTEROP_API int flex_input(IN_NOTNULL scan_t *scan, OUT_WRITES(size) char *out, int size);

/// @brief initialize source location tracking
///
/// @param where the source location to initialize
CT_INTEROP_API void flex_init(IN_NOTNULL where_t *where);

/// @brief update the source location
///
/// @param where the source location to update
/// @param offsets the source location offsets
/// @param steps the number of steps to update by
CT_INTEROP_API void flex_update(IN_NOTNULL where_t *where, IN_READS(steps) const where_t *offsets, int steps);

/// track source locations inside flex and bison
#ifndef YY_USER_ACTION
#   define YY_USER_ACTION flex_action(yylloc, yytext);
#endif

/// read input for flex and bison
#ifndef YY_INPUT
#   define YY_INPUT(buffer, result, size)          \
        result = flex_input(yyextra, buffer, size); \
        if ((result) <= 0)                          \
        {                                           \
            (result) = YY_NULL;                     \
        }
#endif

/// default source location update function
#ifndef YYLLOC_DEFAULT
#   define YYLLOC_DEFAULT(current, rhs, offset) flex_update(&(current), rhs, offset)
#endif

/// initialize flex and bison
#ifndef YY_USER_INIT
#   define YY_USER_INIT flex_init(yylloc);
#endif

/// install our own error handler for nicer error messages
#ifndef YY_FATAL_ERROR
#   define YY_FATAL_ERROR(msg) CT_NEVER("fatal flex error: %s", msg)
#endif

/// @}

CT_END_API
