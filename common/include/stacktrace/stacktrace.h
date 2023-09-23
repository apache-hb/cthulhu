#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STACKTRACE_NAME_LENGTH 256

typedef struct frame_t {
    char name[STACKTRACE_NAME_LENGTH];
} frame_t;

void stacktrace_init(void);
const char *stacktrace_backend(void);

size_t stacktrace_get(frame_t *frames, size_t size);

void stacktrace_print(FILE *file);

#ifdef __cplusplus
}
#endif
