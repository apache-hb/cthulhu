#include "io/io.h"
#include "io/impl.h"

#include "base/panic.h"
#include "std/str.h"

void io_close(io_t *io)
{
    CTASSERT(io != NULL);

    if (io->cb->fn_close != NULL)
        io->cb->fn_close(io);
}

USE_DECL
size_t io_read(io_t *io, void *dst, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->flags & eAccessRead, "cannot io_read(%s). flags did not include eAccessRead", io_name(io));
    CTASSERTF(io->cb->fn_read, "fn_read not provided for `%s`", io->name);

    return io->cb->fn_read(io, dst, size);
}

USE_DECL
size_t io_write(io_t *io, const void *src, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->flags & eAccessWrite, "cannot io_write(%s). flags did not include eAccessWrite", io_name(io));
    CTASSERTF(io->cb->fn_write, "fn_write not provided for `%s`", io->name);

    return io->cb->fn_write(io, src, size);
}

USE_DECL
size_t io_printf(io_t *io, const char *fmt, ...)
{
    CTASSERT(io != NULL);

    va_list args;
    va_start(args, fmt);

    size_t size = io_vprintf(io, fmt, args);

    va_end(args);

    return size;
}

USE_DECL
size_t io_vprintf(io_t *io, const char *fmt, va_list args)
{
    CTASSERT(io != NULL);

    const io_callbacks_t *cb = io->cb;
    if (cb->fn_write_format != NULL)
    {
        return cb->fn_write_format(io, fmt, args);
    }

    text_t text = text_vformat(io->arena, fmt, args);
    return io_write(io, text.text, text.length);
}

USE_DECL
size_t io_size(io_t *io)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->cb->fn_get_size, "fn_get_size not provided for `%s`", io->name);

    return io->cb->fn_get_size(io);
}

USE_DECL
size_t io_seek(io_t *io, size_t offset)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->cb->fn_seek, "fn_seek not provided for `%s`", io->name);

    return io->cb->fn_seek(io, offset);
}

USE_DECL
const char *io_name(const io_t *io)
{
    CTASSERT(io != NULL);

    return io->name;
}

USE_DECL
void *io_map(io_t *io, os_protect_t protect)
{
    CTASSERT(io != NULL);
    CTASSERTF(io->cb->fn_map, "fn_map not provided for `%s`", io->name);

    CTASSERTF(protect != eProtectNone, "cannot io_map(%s). protect is eProtectNone", io_name(io));

    // validate protect against access flags
    if (protect & eProtectRead)
    {
        CTASSERTF(io->flags & eAccessRead, "io.map(%s) flags not readable", io_name(io));
    }

    if (protect & eProtectWrite)
    {
        CTASSERTF(io->flags & eAccessWrite, "io.map(%s) flags not writable", io_name(io));
    }

    if (io_size(io) == 0) { return ""; }

    return io->cb->fn_map(io, protect);
}

USE_DECL
io_error_t io_error(const io_t *io)
{
    CTASSERT(io != NULL);

    return io->error;
}
