#include "base/panic.h"

#include "core/macros.h"

#include "stacktrace/stacktrace.h"

#include <stdio.h>
#include <stdlib.h>

static void default_panic_handler(panic_t panic, const char *fmt, va_list args)
{
    (void)fprintf(stderr, ANSI_CYAN "[panic]" ANSI_RESET "[%s:%zu] => " ANSI_RED "%s" ANSI_RESET ": ", panic.file, panic.line, panic.function);
    (void)vfprintf(stderr, fmt, args);
    (void)fprintf(stderr, "\n");

#if ADDRSAN_ENABLED
    volatile char *ptr = NULL;
    *ptr = 0;
#else
    bt_print_trace(stderr);
#endif
}

panic_handler_t ctu_default_panic(void)
{
    return default_panic_handler;
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