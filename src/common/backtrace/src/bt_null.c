#include "backtrace/backtrace.h"

#include <stdio.h>

void bt_init(void) { }

USE_DECL
const char *bt_backend(void)
{
    return "null";
}

void bt_read_inner(bt_frame_t callback, void *user)
{
    frame_t frame = { 0 };
    callback(user, &frame);
}

void bt_resolve_inner(const frame_t *frame, symbol_t *symbol)
{
    text_t name = symbol->name;
    snprintf(name.text, name.size, "%" PRI_ADDRESS, frame->address);
}
