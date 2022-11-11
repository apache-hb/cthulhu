#include "stacktrace/stacktrace.h"

void stacktrace_init(void) { }

const char *stacktrace_backend(void)
{
    return "null";
}

size_t stacktrace_get(frame_t *frames, size_t size)
{
    (void)frames;
    (void)size;

    return 0;
}
