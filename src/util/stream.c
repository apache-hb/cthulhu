#include "cthulhu/util/stream.h"
#include "base/util.h"

#include <string.h>

stream_t *stream_new(size_t size)
{
    stream_t *out = ctu_malloc(sizeof(stream_t));
    out->size = size;
    out->len = 0;

    out->data = ctu_malloc(size + 1);
    out->data[0] = 0;

    return out;
}

stream_t *stream_of(const char *str)
{
    stream_t *out = stream_new(strlen(str));
    stream_write(out, str);
    return out;
}

void stream_delete(stream_t *stream)
{
    ctu_free(stream->data);
    ctu_free(stream);
}

size_t stream_len(stream_t *stream)
{
    return stream->len;
}

void stream_write(stream_t *stream, const char *str)
{
    stream_write_bytes(stream, str, strlen(str));
}

void stream_write_bytes(stream_t *stream, const void *bytes, size_t len)
{
    if (stream->len + len > stream->size)
    {
        stream->size = stream->len + len;
        stream->data = ctu_realloc(stream->data, stream->size + 1);
    }

    memcpy(stream->data + stream->len, bytes, len);
    stream->len += len;

    stream->data[stream->len] = 0;
}

const char *stream_data(const stream_t *stream)
{
    return stream->data;
}

void stream_reset(stream_t *stream)
{
    stream->len = 0;
    stream->data[0] = 0;
}
