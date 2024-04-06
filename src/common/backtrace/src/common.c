// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"
#include <string.h>

bt_error_t gSystemError = { 0 };

USE_DECL
frame_resolve_t bt_resolve_symbol(bt_address_t frame, bt_symbol_t *symbol)
{
    // inside this function we cant assert
    // because we might be called from inside an assert
    if (symbol == NULL)
        return eResolveNothing;

    text_t name = symbol->name;
    text_t path = symbol->path;

    if (name.text == NULL || path.text == NULL)
        return eResolveNothing;

    symbol->line = CT_LINE_UNKNOWN;
    strcpy_s(name.text, name.length, "<unknown>");
    strcpy_s(path.text, path.length, "<unknown>");

    return bt_resolve_inner(frame, symbol);
}

USE_DECL
void bt_read(bt_trace_t callback, void *user)
{
    if (callback == NULL) return;

    bt_read_inner(callback, user);
}
