#include "io/common.h"

#include "base/memory.h"
#include "base/panic.h"

void io_close(io_t *io)
{
    CTASSERT(io != NULL);

    if (io->cb->fnClose != NULL)
    {
        io->cb->fnClose(io);
    }

    ctu_free(io);
}

USE_DECL
size_t io_read(io_t *io, void *dst, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERT(io->flags & eFileRead);

    return io->cb->fnRead(io, dst, size);
}

USE_DECL
size_t io_write(io_t *io, const void *src, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERT(io->flags & eFileWrite);

    return io->cb->fnWrite(io, src, size);
}

USE_DECL
size_t io_size(io_t *io)
{
    CTASSERT(io != NULL);

    return io->cb->fnGetSize(io);
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

    return io->cb->fnMap(io);
}

USE_DECL
io_error_t io_error(const io_t *io)
{
    CTASSERT(io != NULL);

    return io->error;
}
