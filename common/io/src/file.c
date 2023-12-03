#include "io/impl.h"

#include "base/panic.h"

#include <stdint.h>

typedef struct io_file_t {
    os_file_t *file;
} io_file_t;

static const io_callbacks_t kFileCallbacks;

static os_file_t *fd_data(io_t *self)
{
    io_file_t *file = io_get_data(self, &kFileCallbacks);
    return file->file;
}

static size_t fd_read(io_t *self, void *dst, size_t size)
{
    os_file_t *file = fd_data(self);

    OS_RESULT(size_t) read = os_file_read(file, dst, size);
    self->error = os_error(read);

    return OS_VALUE_OR(size_t, read, SIZE_MAX);
}

static size_t fd_write(io_t *self, const void *src, size_t size)
{
    os_file_t *file = fd_data(self);

    OS_RESULT(size_t) written = os_file_write(file, src, size);
    self->error = os_error(written);

    return OS_VALUE_OR(size_t, written, SIZE_MAX);
}

static size_t fd_size(io_t *self)
{
    os_file_t *file = fd_data(self);

    OS_RESULT(size_t) size = os_file_size(file);
    self->error = os_error(size);

    return OS_VALUE_OR(size_t, size, SIZE_MAX);
}

static size_t fd_seek(io_t *self, size_t offset)
{
    os_file_t *file = fd_data(self);

    OS_RESULT(size_t) seek = os_file_seek(file, offset);
    self->error = os_error(seek);

    return OS_VALUE_OR(size_t, seek, SIZE_MAX);
}

static const void *fd_map(io_t *self)
{
    os_file_t *file = fd_data(self);

    OS_RESULT(const void *) data = os_file_map(file);
    self->error = os_error(data);

    return OS_VALUE_OR(const void *, data, NULL);
}

static void fd_close(io_t *self)
{
    os_file_t *file = fd_data(self);
    os_file_close(file);
}

static const io_callbacks_t kFileCallbacks = {
    .fn_read = fd_read,
    .fn_write = fd_write,

    .fn_get_size = fd_size,
    .fn_seek = fd_seek,

    .fn_map = fd_map,
    .fn_close = fd_close
};

USE_DECL
io_t *io_file(const char *path, os_access_t mode)
{
    OS_RESULT(os_file_t *) file = os_file_open(path, mode);

    os_file_t *fd = os_value(file);
    io_file_t it = { .file = fd };

    io_t *io = io_new(&kFileCallbacks, mode, path, &it, sizeof(io_file_t));
    io->error = os_error(file);

    return io;
}
