// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_interop_api.h>

#include "base/panic.h"

#include "interop/actions.h" // IWYU pragma: export

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
