// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/compiler.h"

/// @defgroup macros Common macros
/// @brief Common macros used throughout the project
/// @ingroup core
/// @{

#ifdef __cplusplus
#   define CT_STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#else
#   define CT_STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)
#endif

#if __cplusplus >= 201402L
#   define CT_DEPRECATED(msg) [[deprecated(msg)]]
#elif CT_CC_MSVC || CT_CC_CLANGCL
#   define CT_DEPRECATED(msg) __declspec(deprecated(msg))
#elif CT_CC_CLANG || CT_CC_GNU
#   define CT_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#   define CT_DEPRECATED(msg)
#endif

/// @def CT_DEPRECATED(msg)
/// @brief mark a function as deprecated
/// @param msg the message to display when the function is used

/// @def CT_MAX(lhs, rhs)
/// returns the maximum of @a lhs and @a rhs
#define CT_MAX(L, R) ((L) > (R) ? (L) : (R))

/// @def CT_MIN(lhs, rhs)
/// returns the minimum of @a lhs and @a rhs
#define CT_MIN(L, R) ((L) < (R) ? (L) : (R))

/// @def CT_ALIGN_POW2(X, ALIGN)
/// aligns @p X to the next power of 2 of @p ALIGN
#define CT_ALIGN_POW2(X, ALIGN) (((X) + (ALIGN)-1) & ~((ALIGN)-1))

/// @def CT_UNUSED(x)
/// @brief mark a variable as unused
#define CT_UNUSED(x) ((void)(x))

#define CT_INNER_STR(x) #x
#define CT_STR(x) CT_INNER_STR(x)

#define CT_VERSION CT_STR(CTU_MAJOR) "." CT_STR(CTU_MINOR) "." CT_STR(CTU_PATCH)

/// @defgroup ansi_colour ANSI Colour macros
/// @brief ANSI escape string colour macros
/// @ingroup core
///
/// Useful for formatting messages to the console.
/// @{

#define CT_ANSI_RED "\x1B[1;31m"    ///< ANSI red
#define CT_ANSI_GREEN "\x1B[1;32m"  ///< ANSI green
#define CT_ANSI_YELLOW "\x1B[1;33m" ///< ANSI yellow
#define CT_ANSI_BLUE "\x1B[1;34m"   ///< ANSI blue
#define CT_ANSI_MAGENTA "\x1B[1;35m" ///< ANSI magenta
#define CT_ANSI_CYAN "\x1B[1;36m"   ///< ANSI cyan
#define CT_ANSI_WHITE "\x1B[1;37m"  ///< ANSI white

#define CT_ANSI_RESET "\x1B[0m"     ///< ANSI reset

/// @}

/// @defgroup error_codes Exit codes
/// @brief Exit codes that match with GNU standard codes
/// @ingroup Core
///
/// Used in tests and the cli.
/// @{

#define CT_EXIT_OK 0        ///< no user errors or internal errors
#define CT_EXIT_ERROR 1     ///< the user has made an error
#define CT_EXIT_SHOULD_EXIT 2 ///< the user has requested to exit the program
#define CT_EXIT_INTERNAL 99 ///< internal compiler errors have occurred

/// @}

/// @}
