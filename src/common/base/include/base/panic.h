#pragma once

#include <ctu_base_api.h>

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdarg.h>
#include <stddef.h>

BEGIN_API

/// @defgroup panic Assertions and panic handling
/// @ingroup base
/// @{

#ifdef WITH_DOXYGEN
#   define CTU_DEBUG 1
#   define CTU_PARANOID 1
#endif

/// @def CTU_DEBUG
/// @brief enable panic handling
/// @note this is only enabled in debug builds, see [The build guide](@ref building) for more
/// information

/// @def CTU_PARANOID
/// @brief enable paranoid assertions
/// for expensive assertions that shouldnt be used too often
/// use these for things that you do not want being turned into assumes due to the execution cost

// TODO: move source_info_t into core
// semanticly wrong for it to be in base

/// @brief panic location information
typedef struct source_info_t
{
    /// @brief the file the panic occurred in
    FIELD_STRING const char *file;

    /// @brief the line the panic occurred on
    size_t line;

    /// @brief the function the panic occurred in
    /// @note this could also be the name of a variable in some uses in c++
    FIELD_STRING const char *function;
} source_info_t;

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
NORETURN CT_BASE_API ctu_panic(source_info_t location, FMT_STRING const char *msg, ...) CT_PRINTF(2, 3);

/// @brief panic with a message, file, and line
///
/// @param location the source location of the panic
/// @param msg the message to panic with
/// @param args the arguments to format
NORETURN CT_BASE_API ctu_vpanic(source_info_t location, FMT_STRING const char *msg, va_list args);

/// @def CT_SOURCE_HERE
/// @brief the source location of the current line
#define CT_SOURCE_HERE {__FILE__, __LINE__, CT_FUNCNAME}

/// @def CT_PANIC(...)
/// @brief panic with a message and optional format arguments
///
/// @param ... the format string and optional arguments to format
#define CT_PANIC(...)                                  \
    do                                                  \
    {                                                   \
        source_info_t panic_source = CT_SOURCE_HERE; \
        ctu_panic(panic_source, __VA_ARGS__);                    \
    } while (0)

/// @def CTASSERTF_ALWAYS(expr, ...)
/// @brief assert a condition with a message and optional format arguments
/// @note this always expands to a panic
///
/// @param expr the condition to assert
/// @param ... the format string and optional arguments to format

#define CTASSERTF_ALWAYS(expr, ...) \
    do                                \
    {                                 \
        if (!(expr))                  \
        {                             \
            CT_PANIC(__VA_ARGS__);   \
        }                             \
    } while (0)

/// @def CTASSERTF(expr, ...)
/// @brief assert a condition with a message and optional format arguments
/// @note in release builds this expands to @a CT_ASSUME
///
/// @param expr the condition to assert
/// @param ... the format string and optional arguments to format

#if CTU_DEBUG
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

/// @def NEVER(...)
/// @brief assert that a code path is never reached
///
/// @param ... the format string and optional arguments to format
#define NEVER(...) CT_PANIC(__VA_ARGS__)

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

END_API
