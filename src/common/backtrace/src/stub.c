// SPDX-License-Identifier: LGPL-3.0-only

#include "backtrace/backtrace.h"

#include <stdio.h>

void bt_init(void) { }
void bt_update(void) { }

USE_DECL
const char *bt_backend(void)
{
    return "null";
}

void bt_read_inner(bt_trace_t callback, void *user)
{
    bt_address_t frame = 0;
    callback(frame, user);
}

frame_resolve_t bt_resolve_inner(bt_address_t frame, bt_symbol_t *symbol)
{
    text_t name = symbol->name;
    (void)snprintf(name.text, name.length, "%" PRI_ADDRESS, frame);

    return eResolveNothing;
}
