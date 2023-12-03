#include "io/impl.h"

#include "memory/memory.h"
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
    CTASSERTF(io->cb->fn_read, "fn_read invalid for `%s`", io->name);

    return io->cb->fn_read(io, dst, size);
}

USE_DECL
size_t io_write(io_t *io, const void *src, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERT(io->flags & eAccessWrite);
    CTASSERTF(io->cb->fn_write, "fn_write invalid for `%s`", io->name);

    return io->cb->fn_write(io, src, size);
}

USE_DECL
size_t io_size(io_t *io)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->cb->fn_get_size, "fn_get_size invalid for `%s`", io->name);

    return io->cb->fn_get_size(io);
}

USE_DECL
size_t io_seek(io_t *io, size_t offset)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->cb->fn_seek, "fn_seek invalid for `%s`", io->name);

    return io->cb->fn_seek(io, offset);
}

USE_DECL
const char *io_name(const io_t *io)
{
    CTASSERT(io != NULL);

    return io->name;
}

USE_DECL
const void *io_map(io_t *io)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->cb->fn_map, "fn_map invalid for `%s`", io->name);

    if (io_size(io) == 0) { return ""; }

    return io->cb->fn_map(io);
}

USE_DECL
io_error_t io_error(const io_t *io)
{
    CTASSERT(io != NULL);

    return io->error;
}
