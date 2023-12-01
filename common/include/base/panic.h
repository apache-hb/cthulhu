#pragma once

#include "base/analyze.h"
#include "base/macros.h"

#include <stdarg.h>
#include <stddef.h>

BEGIN_API

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

#define CTU_PANIC(...)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        panic_t panic = {__FILE__, __LINE__, FUNCNAME};                                                                \
        ctpanic(panic, __VA_ARGS__);                                                                                   \
    } while (0)

#define CTU_ALWAYS_ASSERTF(expr, ...)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            CTU_PANIC(__VA_ARGS__);                                                                                    \
        }                                                                                                              \
    } while (0)

#if CTU_DEBUG
#   define CTASSERTF(expr, ...) CTU_ALWAYS_ASSERTF(expr, __VA_ARGS__)
#else
#   define CTASSERTF(expr, ...) CTU_ASSUME(expr)
#endif

#define CTASSERTM(expr, msg) CTASSERTF(expr, msg)
#define CTASSERT(expr) CTASSERTM(expr, #expr)
#define NEVER(...) CTU_PANIC(__VA_ARGS__)

#define GLOBAL_INIT(ID)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        static bool init = false;                                                                                      \
        CTASSERTM(!init, ID " already initialized");                                                                   \
        init = true;                                                                                                   \
    } while (0)

END_API
