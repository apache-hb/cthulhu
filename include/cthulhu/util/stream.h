#pragma once

#include "cthulhu/util/macros.h"
#include <stddef.h>

typedef struct
{
    size_t len;
    size_t size;
    char *data;
} stream_t;

void stream_delete(stream_t *stream);
stream_t *stream_new(size_t size);
size_t stream_len(stream_t *stream);
void stream_write(stream_t *stream, const char *str);
void stream_write_bytes(stream_t *stream, const void *bytes, size_t len);
const char *stream_data(const stream_t *stream);
