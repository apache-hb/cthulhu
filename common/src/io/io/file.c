#include "common.h"

#include "report/report.h"

static size_t fd_read(io_t *self, void *dst, size_t size)
{
    file_t *file = io_data(self);
    return file_read(*file, dst, size, &self->error);
}

static size_t fd_write(io_t *self, const void *src, size_t size)
{
    file_t *file = io_data(self);
    return file_write(*file, src, size, &self->error);
}

static size_t fd_size(io_t *self)
{
    file_t *file = io_data(self);
    return file_size(*file, &self->error);
}

static size_t fd_seek(io_t *self, size_t offset)
{
    file_t *file = io_data(self);
    return file_seek(*file, offset, &self->error);
}

static const void *fd_map(io_t *self)
{
    file_t *file = io_data(self);
    return file_map(*file, &self->error);
}

static void fd_close(io_t *self)
{
    file_t *file = io_data(self);
    file_close(*file);
}

static const io_callbacks_t kFileCallbacks = {
    .fnRead = fd_read,
    .fnWrite = fd_write,

    .fnGetSize = fd_size,
    .fnSeek = fd_seek,

    .fnMap = fd_map,
    .fnClose = fd_close
};

USE_DECL
io_t *io_file(const char *path, file_flags_t mode)
{
    cerror_t err = 0;
    file_t file = file_open(path, mode, &err);
    io_t *io = io_new(&kFileCallbacks, mode, path, &file, sizeof(file_t));
    io->error = err;
    return io;
}
