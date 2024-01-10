#include "base/panic.h"

#include "backtrace/backtrace.h"

#include <stdio.h>
#include <stdlib.h>

static void default_panic_handler(panic_t panic, const char *fmt, va_list args)
{
    (void)fprintf(stderr, "[panic][%s:%zu] => %s: ", panic.file, panic.line, panic.function);
    (void)vfprintf(stderr, fmt, args);
    (void)fprintf(stderr, "\n");

    bt_print_trace(stderr);
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
