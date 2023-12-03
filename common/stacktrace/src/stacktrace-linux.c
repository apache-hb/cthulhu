#include "stacktrace/stacktrace.h"

#include <execinfo.h>
#include <stdlib.h>
#include <string.h>

void stacktrace_init(void) { }

const char *stacktrace_backend(void)
{
    return "linux-glibc-backtrace";
}

// TODO: this is probably wrong
size_t stacktrace_get(frame_t *frames, size_t size)
{
    void **buffers = malloc(size * sizeof(void*));
    size_t count = (size_t)backtrace(buffers, size);
    char **strings = backtrace_symbols(buffers, size);

    size_t used = count < size ? count : size;

    for (size_t i = 0; i < used; i++)
    {
        strcpy(frames[i].name, strings[i]);
    }

    free(strings);

    return used;
}
