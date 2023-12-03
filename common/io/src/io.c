#include "io/impl.h"

#include "base/memory.h"
#include "base/panic.h"

void io_close(io_t *io)
{
    CTASSERT(io != NULL);

    if (io->cb->fn_close != NULL)
    {
        io->cb->fn_close(io);
    }

    ctu_free(io);
}

USE_DECL
size_t io_read(io_t *io, void *dst, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->flags & eAccessRead, "io.read(%s) flags not readable", io_name(io));

    return io->cb->fn_read(io, dst, size);
}

USE_DECL
size_t io_write(io_t *io, const void *src, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERT(io->flags & eAccessWrite);

    return io->cb->fn_write(io, src, size);
}

USE_DECL
size_t io_size(io_t *io)
{
    CTASSERT(io != NULL);

    return io->cb->fn_get_size(io);
}

USE_DECL
size_t io_seek(io_t *io, size_t offset)
{
    CTASSERT(io != NULL);

    return io->cb->fn_seek(io, offset);
}

USE_DECL
const char *io_name(io_t *io)
{
    CTASSERT(io != NULL);

    return io->name;
}

USE_DECL
const void *io_map(io_t *io)
{
    CTASSERT(io != NULL);

    if (io_size(io) == 0) { return ""; }

    return io->cb->fn_map(io);
}

USE_DECL
io_error_t io_error(const io_t *io)
{
    CTASSERT(io != NULL);

    return io->error;
}
