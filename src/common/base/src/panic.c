#include "base/panic.h"

#include "core/macros.h"

#include <stdlib.h>

static void default_panic_handler(panic_t panic, const char *fmt, va_list args)
{
    CTU_UNUSED(panic);
    CTU_UNUSED(fmt);
    CTU_UNUSED(args);

    exit(EXIT_INTERNAL); // NOLINT(concurrency-mt-unsafe)
}

panic_handler_t gPanicHandler = default_panic_handler;

USE_DECL
void ctpanic(panic_t panic, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    gPanicHandler(panic, msg, args);
    va_end(args);

    exit(99); // NOLINT(concurrency-mt-unsafe)
}
