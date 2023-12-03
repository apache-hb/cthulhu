#pragma once

#include "core/analyze.h"

#include <stdarg.h>
#include <stddef.h>

BEGIN_API

/// @defgroup Panic Assertions and panic handling
/// @{

/// @brief panic location information
typedef struct
{
    /// the file the panic occurred in
    FIELD_STRING const char *file;

    /// the line the panic occurred on
    size_t line;

    /// the function the panic occurred in
    FIELD_STRING const char *function;
} panic_t;

/// @brief panic handler function
/// @param panic the panic information
/// @param fmt the format string
/// @param args the format arguments
///
/// @note this function should not allocate memory using a compiler arena
/// @note this function should not return
typedef void (*panic_handler_t)(panic_t panic, const char *fmt, va_list args);

/// @brief the global panic handler
/// by default this prints a stacktrace and aborts
/// it can be overridden for testing purposes or to add more functionality
extern panic_handler_t gPanicHandler;

/// @brief panic with a message, file, and line
///
/// @param panic the panic information
/// @param msg the message to panic with
/// @param ... the arguments to format
FORMAT_ATTRIB(2, 3)
NORETURN ctpanic(panic_t panic, FORMAT_STRING const char *msg, ...);

/// @def CTU_PANIC(...)
/// @brief panic with a message and optional format arguments
///
/// @param ... the format string and optional arguments to format
#define CTU_PANIC(...)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        panic_t panic = {__FILE__, __LINE__, FUNCNAME};                                                                \
        ctpanic(panic, __VA_ARGS__);                                                                                   \
    } while (0)

/// @def CTU_ALWAYS_ASSERTF(expr, ...)
/// @brief assert a condition with a message and optional format arguments
/// @note this always expands to a panic
///
/// @param expr the condition to assert
/// @param ... the format string and optional arguments to format

#define CTU_ALWAYS_ASSERTF(expr, ...)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            CTU_PANIC(__VA_ARGS__);                                                                                    \
        }                                                                                                              \
    } while (0)

/// @def CTASSERTF(expr, ...)
/// @brief assert a condition with a message and optional format arguments
/// @note in release builds this expands to @a CTU_ASSUME
///
/// @param expr the condition to assert
/// @param ... the format string and optional arguments to format

#if CTU_DEBUG
#   define CTASSERTF(expr, ...) CTU_ALWAYS_ASSERTF(expr, __VA_ARGS__)
#else
#   define CTASSERTF(expr, ...) CTU_ASSUME(expr)
#endif

/// @def CTASSERTM(expr, msg)
/// @brief assert a condition with a message
///
/// @param expr the condition to assert
/// @param msg the message to print
#define CTASSERTM(expr, msg) CTASSERTF(expr, msg)

/// @def CTASSERT(expr)
/// @brief assert a condition, prints the condition as a message
///
/// @param expr the condition to assert
#define CTASSERT(expr) CTASSERTM(expr, #expr)

/// @def NEVER(...)
/// @brief assert that a code path is never reached
///
/// @param ... the format string and optional arguments to format
#define NEVER(...) CTU_PANIC(__VA_ARGS__)

/// @def GLOBAL_INIT(ID)
/// @brief assert that a global is only initialized once
///
/// @param ID the unique identifier for this global
#define GLOBAL_INIT(ID)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        static bool init = false;                                                                                      \
        CTASSERTM(!init, ID " already initialized");                                                                   \
        init = true;                                                                                                   \
    } while (0)

/// @} // Panic

END_API
