#include "stacktrace/stacktrace.h"

#include "core/macros.h"

#define STACK_FRAMES 128

USE_DECL
void stacktrace_print(FILE *file)
{
    fprintf(file, COLOUR_CYAN "stacktrace backend:" COLOUR_RESET " %s\n", stacktrace_backend());

    frame_t frames[STACK_FRAMES] = { 0 };
    size_t count = stacktrace_get(frames, STACK_FRAMES);
    for (size_t i = 0; i < count; i++)
    {
        frame_t *frame = &frames[i];
        fprintf(file, COLOUR_CYAN "[%zu]" COLOUR_RESET ": %s (%s:%zu)\n", i, frame->name, frame->path, frame->line);
    }
}