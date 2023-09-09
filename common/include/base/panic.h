#pragma once

#include "base/analyze.h"
#include "base/macros.h"

#include <stdarg.h>
#include <stddef.h>

typedef struct
{
    const char *file;
    size_t line;
    const char *function;
} panic_t;

/**
 * @brief a panic handler callback
 *
 * @note this function is not allowed to allocate using any allocator the compiler
 *       has used.
 */
typedef void (*panic_handler_t)(panic_t panic, const char *fmt, va_list args);

/**
 * @brief global panic handler
 */
extern panic_handler_t gPanicHandler;

/**
 * @brief panic with a message, file, and line
 *
 * @param panic the panic information
 * @param msg the message to panic with
 * @param ... the arguments to format
 */
FORMAT_ATTRIB(2, 3)
NORETURN ctpanic(panic_t panic, FORMAT_STRING const char *msg, ...);

#define CTU_PANIC(...) do { panic_t panic = {__FILE__, __LINE__, FUNCNAME}; ctpanic(panic, __VA_ARGS__); } while (0)

#if ENABLE_DEBUG
#    define CTASSERTF(expr, ...)                                                                                       \
        do                                                                                                             \
        {                                                                                                              \
            if (!(expr))                                                                                               \
            {                                                                                                          \
                CTU_PANIC(__VA_ARGS__);                                                                         \
            }                                                                                                          \
        } while (0)
#else
#    define CTASSERTF(expr, ...) ASSUME(expr)
#endif

#define CTASSERTM(expr, msg) CTASSERTF(expr, msg)
#define CTASSERT(expr) CTASSERTM(expr, #expr)
#define NEVER(...) CTU_PANIC(__VA_ARGS__)

#define GLOBAL_INIT(ID) do { static bool init = false; CTASSERTM(!init, ID " already initialized"); init = true; } while (0)
