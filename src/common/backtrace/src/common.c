// SPDX-License-Identifier: LGPL-3.0-only

#ifndef _CRT_SECURE_NO_WARNINGS
#   define _CRT_SECURE_NO_WARNINGS
#endif

#include "common.h"

#include <string.h>

bt_error_t gSystemError = { 0 };

STA_DECL
bt_resolve_t bt_resolve_symbol(bt_address_t frame, bt_symbol_t *symbol)
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
    strncpy(name.text, "<unknown>", name.length);
    strncpy(path.text, "<unknown>", path.length);

    return bt_resolve_inner(frame, symbol);
}

STA_DECL
void bt_read(bt_trace_t callback, void *user)
{
    if (callback == NULL) return;

    bt_read_inner(callback, user);
}
