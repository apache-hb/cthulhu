#include "io/impl.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

void *io_get_data(io_t *io, const io_callbacks_t *cb)
{
    CTASSERT(io != NULL);
    CTASSERT(cb != NULL);
    CTASSERTF(io->cb == cb, "io type mismatch in %s", io->name);

    return io->data;
}

io_t *io_new(const io_callbacks_t *cb,
             os_access_t flags,
             const char *name,
             const void *data,
             size_t size)
{
    CTASSERT(cb != NULL);
    CTASSERT(name != NULL);
    CTASSERT(data != NULL);
    CTASSERT(size > 0);

    if (flags & eAccessWrite)
        CTASSERTF(cb->fn_write != NULL, "%s provided no `fn_write` function for a writable object", name);

    if (flags & eAccessRead)
        CTASSERTF(cb->fn_read != NULL, "%s provided no `fn_read` for a readable object", name);

    CTASSERTF(cb->fn_get_size, "%s provided no `fn_get_size`", name);
    CTASSERTF(cb->fn_seek, "%s provided no `fn_seek`", name);
    CTASSERTF(cb->fn_map, "%s provided no `fn_map`", name);

    io_t *io = ctu_malloc(sizeof(io_t) + size);

    memcpy(io->data, data, size);

    io->cb = cb;
    io->name = name;
    io->flags = flags;
    io->error = 0;

    return io;
}
