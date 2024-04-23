// SPDX-License-Identifier: LGPL-3.0-only

#include "base/util.h"
#include "io/impl.h"

#include "base/panic.h"
#include "arena/arena.h"

USE_DECL
void *io_data(io_t *io)
{
    CTASSERT(io != NULL);

    // return past the end of the object to get the user data region.
    // we do this rather than have a flexible array member because
    // C requires structs with flexible arrays are always the last
    // member of a struct, but io_t is a header struct.
    return io->data;
}

USE_DECL
io_t *io_new(const io_callbacks_t *cb, os_access_t flags, const char *name, const void *data, arena_t *arena)
{
    void *buffer = ARENA_MALLOC(sizeof(io_t) + cb->size, name, NULL, arena);
    return io_init(buffer, cb, flags, name, data, arena);
}

USE_DECL
io_t *io_init(void *buffer, const io_callbacks_t *cb, os_access_t flags, const char *name, const void *data, arena_t *arena)
{
    CTASSERT(buffer != NULL);
    CTASSERT(cb != NULL);
    CTASSERT(name != NULL);

    if (flags & eOsAccessWrite)
        CTASSERTF(cb->fn_write != NULL, "%s provided no `fn_write` function for a writable object",
                  name);

    if (flags & eOsAccessRead)
        CTASSERTF(cb->fn_read != NULL, "%s provided no `fn_read` for a readable object", name);

    io_t *io = buffer;

    io->cb = cb;
    io->name = name;
    io->flags = flags;
    io->error = 0;
    io->arena = arena;

    if (cb->size > 0)
    {
        CTASSERT(data != NULL);
        ctu_memcpy(io_data(io), data, cb->size);
    }

    return io;
}
