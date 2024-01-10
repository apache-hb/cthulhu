#include "common.h"
#include "core/macros.h"

#include <stdlib.h>
#include <string.h>

static void *bt_error_begin(size_t error)
{
    (void)fprintf(stderr, "a fatal error has occured 0x%zX\n", error);
    return NULL;
}

static void bt_error_frame(void *user, const frame_t *frame)
{
    CTU_UNUSED(user);

    char name_buffer[256] = { 0 };
    char path_buffer[512] = { 0 };

    symbol_t symbol = {
        .name = text_make(name_buffer, sizeof(name_buffer)),
        .path = text_make(path_buffer, sizeof(path_buffer))
    };

    frame_resolve_t resolve = bt_resolve_symbol(frame, &symbol);

    text_t name = symbol.name;
    text_t path = symbol.path;

    if (resolve & (eResolveLine | eResolveFile))
    {
        (void)fprintf(stderr, "%s (%s:%zu)\n", name.text, path.text, symbol.line);
    }
    else
    {
        (void)fprintf(stderr, "%s\n", name.text);
    }
}

static void bt_error_end(void *user)
{
    CTU_UNUSED(user);
    exit(EXIT_INTERNAL); // NOLINT(concurrency-mt-unsafe)
}

bt_error_t gErrorReport = {
    .begin = bt_error_begin,
    .end = bt_error_end,
    .frame = bt_error_frame,
};

frame_resolve_t bt_resolve_symbol(const frame_t *frame, symbol_t *symbol)
{
    if (frame == NULL) return eResolveNothing;
    if (symbol == NULL) return eResolveNothing;

    text_t name = symbol->name;
    text_t path = symbol->path;

    if (name.text == NULL || path.text == NULL)
        return eResolveNothing;

    return bt_resolve_inner(frame, symbol);
}

void bt_read(bt_frame_t callback, void *user)
{
    bt_read_inner(callback, user);
}

typedef struct info_t
{
    symbol_t symbol;
    size_t index;
} info_t;

static void bt_frame(void *user, const frame_t *frame)
{
    info_t *info = user;
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

    info_t info = {
        .symbol = symbol,
        .index = 0
    };

    bt_read(bt_frame, &info);
}
