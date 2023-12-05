#pragma once

#include "core/compiler.h"

#ifdef __cplusplus
#   define STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#else
#   define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)
#endif

#if __cplusplus >= 201402L
#   define DEPRECATED(msg) [[deprecated(msg)]]
#elif CC_MSVC
#   define CTU_DEPRECATED(msg) __declspec(deprecated(msg))
#elif CC_CLANG || CC_GNU
#   define CTU_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#   define CTU_DEPRECATED(msg)
#endif

/// @def MAX(lhs, rhs)
/// returns the maximum of @a lhs and @a rhs
#define MAX(L, R) ((L) > (R) ? (L) : (R))

/// @def MIN(lhs, rhs)
/// returns the minimum of @a lhs and @a rhs
#define MIN(L, R) ((L) < (R) ? (L) : (R))

#define CTU_UNUSED(x) ((void)(x))

#define INNER_STR(x) #x
#define STR(x) INNER_STR(x)

/// @defgroup Colour ANSI Colour macros
/// @brief ANSI escape string colour macros
///
/// Useful for formatting messages to the console.
/// @{

#define COLOUR_RED "\x1B[1;31m"    ///< ANSI escape string for red
#define COLOUR_GREEN "\x1B[1;32m"  ///< ANSI escape string for green
#define COLOUR_YELLOW "\x1B[1;33m" ///< ANSI escape string for yellow
#define COLOUR_BLUE "\x1B[1;34m"   ///< ANSI escape string for blue
#define COLOUR_PURPLE "\x1B[1;35m" ///< ANSI escape string for purple
#define COLOUR_CYAN "\x1B[1;36m"   ///< ANSI escape string for cyan
#define COLOUR_RESET "\x1B[0m"     ///< ANSI escape reset

/// @}

/// @defgroup ErrorCodes Exit codes that match with GNU standard codes
/// @{

#define EXIT_OK 0        ///< no user errors or internal errors
#define EXIT_ERROR 1     ///< the user has made an error
#define EXIT_INTERNAL 99 ///< internal compiler errors have occurred

/// @}

#ifndef __has_feature
#   define __has_feature(...) 0
#endif

#define ADDRSAN_ENABLED ((__SANITIZE_ADDRESS__ != 0) || __has_feature(address_sanitizer))
