#include "backtrace/backtrace.h"

#include <stdio.h>

void bt_init(void) { }

USE_DECL
const char *bt_backend(void)
{
    return "null";
}

void bt_read_inner(bt_trace_t callback, void *user)
{
    bt_frame_t frame = { 0 };
    callback(&frame, user);
}

frame_resolve_t bt_resolve_inner(const bt_frame_t *frame, bt_symbol_t *symbol)
{
    text_t name = symbol->name;
    (void)snprintf(name.text, name.size, "%" PRI_ADDRESS, frame->address);

    return eResolveNothing;
}
