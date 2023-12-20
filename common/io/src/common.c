#include "io/impl.h"

#include "base/panic.h"
#include "memory/memory.h"

#include <string.h>

void *io_data(io_t *io)
{
    CTASSERT(io != NULL);

    return io->data;
}

io_t *io_new(const io_callbacks_t *cb, os_access_t flags, const char *name, const void *data,
             size_t size)
{
    CTASSERT(cb != NULL);
    CTASSERT(name != NULL);

    if (flags & eAccessWrite)
        CTASSERTF(cb->fn_write != NULL, "%s provided no `fn_write` function for a writable object",
                  name);

    if (flags & eAccessRead)
        CTASSERTF(cb->fn_read != NULL, "%s provided no `fn_read` for a readable object", name);

    io_t *io = MEM_ALLOC(sizeof(io_t) + size, name, NULL);

    if (size > 0)
    {
        CTASSERT(data != NULL);
        memcpy(io->data, data, size);
    }

    io->cb = cb;
    io->name = name;
    io->flags = flags;
    io->error = 0;

    return io;
}
