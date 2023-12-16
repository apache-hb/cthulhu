#include "common.h"

#include "core/macros.h"
#include <string.h>

#define STACK_FRAMES 128

void frame_resolve(const frame_t *frame, symbol_t *symbol)
{
    if (frame == NULL) return;
    if (symbol == NULL) return;

    frame_resolve_inner(frame, symbol);
}

typedef struct bt_info_t
{
    frame_t *frames;
    size_t size;
    size_t used;
} bt_info_t;

static void bt_frame(void *user, const frame_t *frame)
{
    bt_info_t *info = user;

    if (info->used >= info->size) return;

    info->frames[info->used++] = *frame;
}

USE_DECL
size_t stacktrace_get(frame_t *frames, size_t size)
{
    if (frames == NULL) return 0;
    if (size == 0) return 0;

    bt_info_t info = {
        .frames = frames,
        .size = size,
        .used = 0
    };

    stacktrace_read(bt_frame, &info);

    return info.used;
}

void stacktrace_read(bt_frame_t callback, void *user)
{
    stacktrace_read_inner(callback, user);
}

USE_DECL
void stacktrace_print(FILE *file)
{
    fprintf(file, ANSI_CYAN "stacktrace backend:" ANSI_RESET " %s", stacktrace_backend());

    symbol_t symbol = { 0 };

    frame_t frames[STACK_FRAMES] = { 0 };
    size_t count = stacktrace_get(frames, STACK_FRAMES);
    for (size_t i = 0; i < count; i++)
    {
        frame_resolve(&frames[i], &symbol);
        fprintf(file, "\n" ANSI_CYAN "[%zu]" ANSI_RESET ": %s (%s:%zu)", i, symbol.name, symbol.file, symbol.line);
    }
}
