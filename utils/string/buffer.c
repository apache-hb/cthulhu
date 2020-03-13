#include "buffer.h"

#include <stdlib.h>

static buffer_t* buffer_with_size(size_t len)
{
    char* data = malloc(len + 1);
    data[len] = '\0';
    buffer_t* buf = malloc(sizeof(buffer_t));
    buf->data = data;
    buf->size = len;
    buf->allocated = len;
    return buf;
}

buffer_t* buffer_new()
{
    return buffer_with_size(64);
}

void buffer_push(buffer_t* self, char c)
{
    if(self->size += 1 >= self->allocated)
        self->data = realloc(self->data, self->allocated += 128);

    memset(self->data + self->size, 0, self->allocated - self->size);

    self->data[self->size] = c;
    self->data[self->size+1] = '\0';
}

buffer_t* buffer_slice(buffer_t* self, size_t begin, size_t end)
{
    buffer_t* out = buffer_with_size(end - begin);
    strncpy(out->data, self->data + begin, end - begin);
    return out;
}
