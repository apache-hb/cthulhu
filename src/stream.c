#include "stream.h"

#include <stdio.h>

stream_t stream_fopen(const char* path)
{
    return stream_new(fopen(path, "rt"), (pfn_stream_next)fgetc);
}

stream_t stream_new(void* data, pfn_stream_next next)
{
    stream_t out = {
        .data = data,
        .next = next,
        .pos = { 0, 0, 0 },
        .ahead = next(data)
    };

    return out;
}

int stream_next(stream_t* self)
{
    int c = self->ahead;
    self->ahead = self->next(self->data);

    if(c == '\n')
    {
        self->pos.col = 0;
        self->pos.line++;
    }
    else
    {
        self->pos.col++;
    }
    self->pos.dist++;

    return c;
}

int stream_peek(stream_t* self)
{
    return self->ahead;
}

int stream_consume(stream_t* self, int c)
{
    if(stream_peek(self) == c)
    {
        stream_next(self);
        return 1;
    }

    return 0;
}
