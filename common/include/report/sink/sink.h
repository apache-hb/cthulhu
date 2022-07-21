#pragma once

typedef struct vector_t vector_t;
typedef struct sink_t sink_t;

typedef void(*sink_flush_t)(sink_t *sink, vector_t *messages);

typedef struct sink_t {
    sink_flush_t flush;
    void *settings;
} sink_t;
