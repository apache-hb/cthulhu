#include "common.h"

#include "core/macros.h"
#include <string.h>

#define STACK_FRAMES 128

void frame_resolve(const frame_t *frame, symbol_t *symbol)
{
    if (frame == NULL) return;
    if (symbol == NULL) return;

    frame_resolve_inner(frame, symbol);

    const size_t len = sizeof(CTU_SOURCE_ROOT);

    if (strncmp(symbol->file, CTU_SOURCE_ROOT, len - 1) == 0)
    {
        strcpy_s(symbol->file, STACKTRACE_PATH_LENGTH, symbol->file + len);
    }
}

USE_DECL
void stacktrace_print(FILE *file)
{
    fprintf(file, COLOUR_CYAN "stacktrace backend:" COLOUR_RESET " %s", stacktrace_backend());

    symbol_t symbol = { 0 };

    frame_t frames[STACK_FRAMES] = { 0 };
    size_t count = stacktrace_get(frames, STACK_FRAMES);
    for (size_t i = 0; i < count; i++)
    {
        frame_resolve(&frames[i], &symbol);
        fprintf(file, "\n" COLOUR_CYAN "[%zu]" COLOUR_RESET ": %s (%s:%zu)", i, symbol.name, symbol.file, symbol.line);
    }
}
