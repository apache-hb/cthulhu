#include "base/panic.h"

#include "stacktrace/stacktrace.h"

#include <stdio.h>
#include <stdlib.h>

#if ADDRSAN_ENABLED
#    include <signal.h>
#endif

#define STACK_FRAMES 128

static void default_panic_handler(panic_t panic, const char *fmt, va_list args)
{
    fprintf(stderr, COLOUR_CYAN "[panic]" COLOUR_RESET "[%s:%zu] => " COLOUR_RED "%s" COLOUR_RESET ": ", panic.file,
            panic.line, panic.function);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    fprintf(stderr, COLOUR_CYAN "stacktrace backend:" COLOUR_RESET " %s\n", stacktrace_backend());

    frame_t frames[STACK_FRAMES] = { 0 };
    size_t count = stacktrace_get(frames, STACK_FRAMES);
    for (size_t i = 0; i < count; i++)
    {
        fprintf(stderr, COLOUR_CYAN "[%zu]" COLOUR_RESET ": %s\n", i, frames[i].name);
    }
}

panic_handler_t globalPanicHandler = default_panic_handler;

USE_DECL
void ctpanic(panic_t panic, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    globalPanicHandler(panic, msg, args);
    va_end(args);

#if ADDRSAN_ENABLED
    raise(SIGSEGV);
#endif

    abort();
}
