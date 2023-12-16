#include "stacktrace/stacktrace.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

void stacktrace_init(void) { }

USE_DECL
const char *stacktrace_backend(void)
{
    return "libunwind";
}

void stacktrace_read_inner(bt_frame_t callback, void *user)
{
    unw_cursor_t cursor;
    unw_context_t ctx;
    unw_word_t ip, sp;

    unw_getcontext(&ctx);
    unw_init_local(&cursor, &ctx);

    while (unw_step(&cursor) > 0)
    {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        frame_t frame = {
            .address = ip
        };

        callback(user, &frame);
    }
}

void frame_resolve_inner(const frame_t *frame, symbol_t *symbol)
{
    snprintf(symbol->name, sizeof(symbol->name), "0x%016" PRIxPTR, frame->address);
}
