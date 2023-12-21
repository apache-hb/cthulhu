#include "io/impl.h"

#include <stdint.h>

/// @brief a wrapper around an os file
typedef struct io_file_t
{
    os_file_t *file; ///< the file
} io_file_t;

static os_file_t *fd_data(io_t *self)
{
    io_file_t *file = io_data(self);
    return file->file;
}

static size_t fd_read(io_t *self, void *dst, size_t size)
{
    os_file_t *file = fd_data(self);

    size_t read = 0;
    self->error = os_file_read(file, dst, size, &read);
    if (self->error != 0) return SIZE_MAX;

    return read;
}

static size_t fd_write(io_t *self, const void *src, size_t size)
{
    os_file_t *file = fd_data(self);

    size_t written = 0;
    self->error = os_file_write(file, src, size, &written);
    if (self->error != 0) return SIZE_MAX;

    return written;
}

static size_t fd_size(io_t *self)
{
    os_file_t *file = fd_data(self);

    size_t size = 0;
    self->error = os_file_size(file, &size);
    if (self->error != 0) return SIZE_MAX;

    return size;
}

static size_t fd_seek(io_t *self, size_t offset)
{
    os_file_t *file = fd_data(self);

    size_t seek = 0;
    self->error = os_file_seek(file, offset, &seek);
    if (self->error != 0) return SIZE_MAX;

    return seek;
}

static const void *fd_map(io_t *self)
{
    os_file_t *file = fd_data(self);

    const void *data = NULL;
    self->error = os_file_map(file, &data);
    if (self->error != 0) return NULL;

    return data;
}

static void fd_close(io_t *self)
{
    os_file_t *file = fd_data(self);
    if (io_error(self) == 0)
    {
        os_file_close(file);
    }
}

static const io_callbacks_t kFileCallbacks = {
    .fn_read = fd_read,
    .fn_write = fd_write,

    .fn_get_size = fd_size,
    .fn_seek = fd_seek,

    .fn_map = fd_map,
    .fn_close = fd_close,
};

USE_DECL
io_t *io_file(const char *path, os_access_t mode, arena_t *arena)
{
    os_file_t *fd = NULL;
    os_error_t err = os_file_open(path, mode, &fd);

    io_file_t it = { .file = fd };

    io_t *io = io_new(&kFileCallbacks, mode, path, &it, sizeof(io_file_t), arena);
    io->error = err;

    return io;
}
