// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_base_api.h>
#include <ctu_config.h>

#include "core/source_info.h"

#include <stdarg.h>

CT_BEGIN_API

/// @defgroup panic Assertions and panic handling
/// @ingroup base
/// @{

/// @def CTU_DEBUG
/// @brief enable panic handling
/// @note this is only enabled in debug builds, see [The build guide](@ref building) for more
/// information

/// @def CTU_PARANOID
/// @brief enable paranoid assertions
/// for expensive assertions that shouldnt be used too often
/// use these for things that you do not want being turned into assumes due to the execution cost

/// @brief panic handler function
/// @param location the source location of the panic
/// @param fmt the format string
/// @param args the format arguments
///
/// @note this function should not allocate memory using a compiler arena
/// @note this function should not return
typedef void (*panic_handler_t)(source_info_t location, const char *fmt, va_list args);

/// @brief the global panic handler.
///
/// by default this prints a stacktrace and aborts
/// it can be overridden for testing purposes or to add more functionality
CT_BASE_API extern panic_handler_t gPanicHandler;

/// @brief panic with a message, file, and line
///
/// @param location the source location of the panic
/// @param msg the message to panic with
/// @param ... the arguments to format
CT_NORETURN CT_BASE_API ctu_panic(source_info_t location, CT_FMT_STRING const char *msg, ...)
    CT_PRINTF(2, 3);

/// @brief panic with a message, file, and line
///
/// @param location the source location of the panic
/// @param msg the message to panic with
/// @param args the arguments to format
CT_NORETURN CT_BASE_API ctu_vpanic(source_info_t location, CT_FMT_STRING const char *msg,
                                   va_list args);

/// @def CT_PANIC(...)
/// @brief panic with a message and optional format arguments
///
/// @param ... the format string and optional arguments to format
#define CT_PANIC(...)                                   \
    do                                                  \
    {                                                   \
        source_info_t panic_source = CT_SOURCE_CURRENT; \
        ctu_panic(panic_source, __VA_ARGS__);           \
    } while (0)

/// @def CTASSERTF_ALWAYS(expr, ...)
/// @brief assert a condition with a message and optional format arguments
/// @note this always expands to a panic
///
/// @param expr the condition to assert
/// @param ... the format string and optional arguments to format

#define CTASSERTF_ALWAYS(expr, ...) \
    do                              \
    {                               \
        if (!(expr))                \
        {                           \
            CT_PANIC(__VA_ARGS__);  \
        }                           \
    } while (0)

/// @def CTASSERTF(expr, ...)
/// @brief assert a condition with a message and optional format arguments
/// @note in release builds this expands to @a CT_ASSUME
///
/// @param expr the condition to assert
/// @param ... the format string and optional arguments to format

#if CTU_ASSERTS
#   define CTASSERTF(expr, ...) CTASSERTF_ALWAYS(expr, __VA_ARGS__)
#else
#   define CTASSERTF(expr, ...) CT_ASSUME(expr)
#endif

/// @def CTASSERTM(expr, msg)
/// @brief assert a condition with a message
///
/// @param expr the condition to assert
/// @param msg the message to print
#define CTASSERTM(expr, msg) CTASSERTF(expr, "%s", msg)

/// @def CTASSERT(expr)
/// @brief assert a condition, prints the condition as a message
///
/// @param expr the condition to assert
#define CTASSERT(expr) CTASSERTM(expr, #expr)

/// @def CT_NEVER(...)
/// @brief assert that a code path is never reached
///
/// @param ... the format string and optional arguments to format
#define CT_NEVER(...) CT_PANIC(__VA_ARGS__)

/// @def CT_ASSERT_RANGE(value, min, max)
/// @brief assert that a value is in a range
/// inclusive bounds check
///
/// @param value the value to check
/// @param min the minimum value
/// @param max the maximum value
#define CT_ASSERT_RANGE(value, min, max) \
    CTASSERTF((value) >= (min) && (value) <= (max), "value %d not in range %d-%d", (value), (min), (max))

#if CTU_PARANOID
#   define CT_PARANOID(...) __VA_ARGS__
#   define CT_PARANOID_ASSERTF(expr, ...) CTASSERTF(expr, __VA_ARGS__)
#else
#   define CT_PARANOID(...)
#   define CT_PARANOID_ASSERTF(expr, ...)
#endif

/// @def CT_PARANOID(...)
/// @brief a block of code that is only executed in paranoid builds

/// @def CT_PARANOID_ASSERTF(expr, ...)
/// @brief assert a condition with a message and optional format arguments

/// @} // Panic

CT_END_API
