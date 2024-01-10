#pragma once

#include "scan/node.h"

#include "base/panic.h"

BEGIN_API

/// @defgroup FlexBisonMacros Helpers for flex and bison driver frontends
/// @ingroup Interop
/// @{

/// @brief tracks the current source position
/// @see https://ftp.gnu.org/old-gnu/Manuals/flex-2.5.4/html_node/flex_14.html
///
/// @param where a pointer to the current location
/// @param text current source text
void flex_action(where_t *where, const char *text);

/// @brief retrevies more input for flex
///
/// @param scan the source scanner
/// @param out output buffer to write to
/// @param size total number of characters to write
///
/// @return number of characters written
int flex_input(scan_t *scan, char *out, int size);

/// @brief initialize source location tracking
///
/// @param where the source location to initialize
void flex_init(where_t *where);

/// @brief update the source location
///
/// @param where the source location to update
/// @param offsets the source location offsets
/// @param steps the number of steps to update by
void flex_update(where_t *where, where_t *offsets, int steps);

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
#   define YY_FATAL_ERROR(msg) NEVER("fatal flex error: %s", msg)
#endif

/// @}

END_API
