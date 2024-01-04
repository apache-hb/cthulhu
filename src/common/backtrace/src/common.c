#include "common.h"

#include <string.h>

frame_resolve_t bt_resolve_symbol(const frame_t *frame, symbol_t *symbol)
{
    if (frame == NULL) return false;
    if (symbol == NULL) return false;

    return bt_resolve_inner(frame, symbol);
}

void bt_read(bt_frame_t callback, void *user)
{
    bt_read_inner(callback, user);
}

typedef struct bt_info_t
{
    symbol_t symbol;
    size_t index;
} bt_info_t;

static void bt_frame(void *user, const frame_t *frame)
{
    bt_info_t *info = user;
    symbol_t *symbol = &info->symbol;

    bt_resolve_symbol(frame, symbol);

    text_t name = symbol->name;
    text_t path = symbol->path;

    (void)fprintf(stderr, "\n [%zu]: %s (%s:%zu)", info->index, name.text, path.text, symbol->line);

    info->index += 1;
}

USE_DECL
void bt_print_trace(FILE *file)
{
    (void)fprintf(file, "stacktrace: %s", bt_backend());

    char name_buffer[256] = { 0 };
    char path_buffer[512] = { 0 };

    symbol_t symbol = {
        .name = text_make(name_buffer, sizeof(name_buffer)),
        .path = text_make(path_buffer, sizeof(path_buffer))
    };

    bt_info_t info = {
        .symbol = symbol,
        .index = 0
    };

    bt_read(bt_frame, &info);
}
