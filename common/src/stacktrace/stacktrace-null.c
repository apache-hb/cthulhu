#include "stacktrace/stacktrace.h"

void stacktrace_init(void) { }

USE_DECL
const char *stacktrace_backend(void)
{
    return "null";
}

USE_DECL
size_t stacktrace_get(frame_t *frames, size_t size)
{
    (void)frames;
    (void)size;

    return 0;
}
