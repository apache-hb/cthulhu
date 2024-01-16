#include "common.h"
#include "core/macros.h"

#include <stdlib.h>
#include <string.h>

static void bt_error_begin(size_t error, void *user)
{
    CTU_UNUSED(error);
    CTU_UNUSED(user);
}

static void bt_error_next(const bt_frame_t *frame, void *user)
{
    CTU_UNUSED(user);
    CTU_UNUSED(frame);
}

static void bt_error_end(void *user)
{
    CTU_UNUSED(user);
    exit(CT_EXIT_INTERNAL); // NOLINT(concurrency-mt-unsafe)
}

bt_error_t gSystemError = {
    .begin = bt_error_begin,
    .end = bt_error_end,
    .next = bt_error_next,
    .user = NULL
};

USE_DECL
frame_resolve_t bt_resolve_symbol(const bt_frame_t *frame, bt_symbol_t *symbol)
{
    // we return nothing rather than asserting because we might be called from inside an assert
    if (frame == NULL) return eResolveNothing;
    if (symbol == NULL) return eResolveNothing;

    text_t name = symbol->name;
    text_t path = symbol->path;

    if (name.text == NULL || path.text == NULL)
        return eResolveNothing;

    return bt_resolve_inner(frame, symbol);
}

USE_DECL
void bt_read(bt_trace_t callback, void *user)
{
    if (callback == NULL) return;

    bt_read_inner(callback, user);
}
