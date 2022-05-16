#pragma once

#include "cthulhu/util/analyze.h"

#define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)

#define UNREACHABLE() ASSUME(false)

/// macros with functionality
#define MAX(L, R) ((L) > (R) ? (L) : (R))
#define MIN(L, R) ((L) < (R) ? (L) : (R))

/**
 * @def MAX(lhs, rhs)
 * returns the maximum of @a lhs and @a rhs
 *
 * @def MIN(lhs, rhs)
 * returns the minimum of @a lhs and @a rhs
 */

/// macros for readability
#define UNUSED(x) ((void)(x))

#define INNER_STR(x) #x
#define STR(x) INNER_STR(x)

/**
 * @defgroup ColourMacros ANSI escape string colour macros
 * @brief ANSI escape string colour macros
 *
 * These are useful for formatting messages to the console.
 * @{
 */

#define COLOUR_RED "\x1B[1;31m"    ///< ANSI escape string for red
#define COLOUR_GREEN "\x1B[1;32m"  ///< ANSI escape string for green
#define COLOUR_YELLOW "\x1B[1;33m" ///< ANSI escape string for yellow
#define COLOUR_BLUE "\x1B[1;34m"   ///< ANSI escape string for blue
#define COLOUR_PURPLE "\x1B[1;35m" ///< ANSI escape string for purple
#define COLOUR_CYAN "\x1B[1;36m"   ///< ANSI escape string for cyan
#define COLOUR_RESET "\x1B[0m"     ///< ANSI escape reset

/** @} */

/// macros for headers
#ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 200112L
#endif

NORETURN ctpanic(FORMAT_STRING const char *msg, ...) FORMAT_ATTRIBUTE(1, 2);

#if ENABLE_DEBUG
#    define CTASSERT(expr, msg)                                                                                        \
        do                                                                                                             \
        {                                                                                                              \
            if (!(expr))                                                                                               \
            {                                                                                                          \
                ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" __FILE__ ":" STR(__LINE__) "]: " msg "\n");             \
            }                                                                                                          \
        } while (0)
#    define CTASSERTF(expr, msg, ...)                                                                                  \
        do                                                                                                             \
        {                                                                                                              \
            if (!(expr))                                                                                               \
            {                                                                                                          \
                ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" __FILE__ ":" STR(__LINE__) "]: " msg "\n",              \
                        __VA_ARGS__);                                                                                  \
            }                                                                                                          \
        } while (0)
#    define union struct
#else
#    define CTASSERT(expr, msg)                                                                                        \
        do                                                                                                             \
        {                                                                                                              \
        } while (0)
#    define CTASSERTF(expr, msg, ...)                                                                                  \
        do                                                                                                             \
        {                                                                                                              \
        } while (0)
#endif
