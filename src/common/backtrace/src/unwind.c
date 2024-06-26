// SPDX-License-Identifier: LGPL-3.0-only

#include "backtrace/backtrace.h"
#include <stdio.h>

#define UNW_LOCAL_ONLY
#include <unwind.h>

static unw_context_t gContext;

void bt_init(void)
{
    unw_getcontext(&gContext);
}

void bt_update(void)
{

}

STA_DECL
const char *bt_backend(void)
{
    return "libunwind";
}

void bt_read_inner(bt_trace_t callback, void *user)
{
    unw_cursor_t cursor;
    unw_word_t ip, sp;

    unw_init_local(&cursor, &gContext);

    while (unw_step(&cursor) > 0)
    {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        bt_address_t frame = ip;

        callback(frame, &frame);
    }
}

bt_resolve_t bt_resolve_inner(bt_address_t frame, bt_symbol_t *symbol)
{
    text_t name = symbol->name;
    (void)snprintf(name.text, name.length, "0x%" PRIxPTR, frame);

    return eResolveNothing;
}
