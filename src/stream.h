#ifndef STREAM_H
#define STREAM_H

#include <stdint.h>

typedef struct {
    uint64_t dist;
    uint64_t col;
    uint64_t line;
} streampos_t;

typedef int(*pfn_stream_next)(void*);

typedef struct {
    void* data;
    streampos_t pos;
    int ahead;
    pfn_stream_next next;
} stream_t;

stream_t stream_fopen(const char* path);
stream_t stream_new(void* data, pfn_stream_next next);

int stream_next(stream_t* self);
int stream_peek(stream_t* self);
int stream_consume(stream_t* self, int c);

#endif // STREAM_H
