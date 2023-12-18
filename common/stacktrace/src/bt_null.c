#include "stacktrace/stacktrace.h"

#include <inttypes.h>

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
    snprintf(symbol->name, sizeof(symbol->name), "0x%016" PRIxPTR, frame->address);
}
