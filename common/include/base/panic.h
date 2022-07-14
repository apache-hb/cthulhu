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

NORETURN ctpanic(panic_t panic, FORMAT_STRING const char *msg, ...) FORMAT_ATTRIBUTE(2, 3);

extern panic_handler_t globalPanicHandler;

#if ENABLE_DEBUG
#    define CTASSERTF(expr, ...)                                                                                       \
        do                                                                                                             \
        {                                                                                                              \
            if (!(expr))                                                                                               \
            {                                                                                                          \
                panic_t panic = {__FILE__, __LINE__, FUNCNAME};                                                        \
                ctpanic(panic, __VA_ARGS__);                                                                           \
            }                                                                                                          \
        } while (0)
#else
#    define CTASSERTF(expr, ...) ASSUME(expr)
#endif

#define CTASSERTM(expr, msg) CTASSERTF(expr, msg)
#define CTASSERT(expr) CTASSERTM(expr, #expr)
