#include "base/panic.h"

panic_handler_t gPanicHandler = NULL;

USE_DECL
void ctu_panic(source_info_t location, const char *msg, ...)
{
    // todo: figure out a nice way to handle assertions that assert themselves
    // we want to catch that and print a nice message rather than overflowing the stack.
    // but i cant think of any way of doing it that would work with green threads (where thread_local doesnt work)
    va_list args;
    va_start(args, msg);
    ctu_vpanic(location, msg, args);
}

USE_DECL
void ctu_vpanic(source_info_t location, CT_FMT_STRING const char *msg, va_list args)
{
    gPanicHandler(location, msg, args);
    CT_ASSUME(0);
}
