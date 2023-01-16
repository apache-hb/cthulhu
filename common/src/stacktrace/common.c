#include "stacktrace/stacktrace.h"

#include "base/macros.h"

#define STACK_FRAMES 128

void stacktrace_print(FILE *file)
{
    fprintf(file, COLOUR_CYAN "stacktrace backend:" COLOUR_RESET " %s\n", stacktrace_backend());

    frame_t frames[STACK_FRAMES] = { 0 };
    size_t count = stacktrace_get(frames, STACK_FRAMES);
    for (size_t i = 0; i < count; i++)
    {
        fprintf(file, COLOUR_CYAN "[%zu]" COLOUR_RESET ": %s\n", i, frames[i].name);
    }
}
