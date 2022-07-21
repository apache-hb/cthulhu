#pragma once

#include <stdbool.h>

typedef struct alloc_t alloc_t;
typedef struct vector_t vector_t;
typedef struct sink_t sink_t;

typedef void(*sink_flush_t)(sink_t *sink, vector_t *messages);

typedef struct sink_t {
    alloc_t *alloc;

    sink_flush_t flush;
    void *settings;
} sink_t;

sink_t *terminal_sink(alloc_t *alloc, bool enableColour);
