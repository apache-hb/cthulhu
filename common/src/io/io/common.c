#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

void *io_data(io_t *io)
{
    CTASSERT(io != NULL);

    return io->data;
}

io_t *io_new(const io_callbacks_t *cb, 
             file_flags_t flags, 
             const char *name, 
             void *data,
             size_t size)
{
    CTASSERT(cb != NULL);
    CTASSERT(name != NULL);
    CTASSERT(data != NULL);
    CTASSERT(size > 0);

    io_t *io = ctu_malloc(sizeof(io_t) + size);

    memcpy(io->data, data, size);
    
    io->cb = cb;
    io->name = name;
    io->flags = flags;
    io->error = 0;

    return io;
}