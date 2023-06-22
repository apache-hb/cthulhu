#include "common.h"

#include "report/report.h"

#include "base/panic.h"

typedef struct io_file_t
{
    os_file_t *file;
} io_file_t;

static os_file_t *fd_data(io_t *self)
{
    io_file_t *file = io_data(self);
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
    .fnRead = fd_read,
    .fnWrite = fd_write,

    .fnGetSize = fd_size,
    .fnSeek = fd_seek,

    .fnMap = fd_map,
    .fnClose = fd_close
};

USE_DECL
io_t *io_file(const char *path, os_access_t mode)
{
    OS_RESULT(os_file_t *) file = os_file_open(path, mode);
    os_file_t *fd = os_value(file);
    CTASSERT(fd != NULL);
    logverbose("file = %p, fd = %p, diff = %zu", file, fd, (uintptr_t)fd - (uintptr_t)file);
    io_file_t it = { .file = fd };
    
    io_t *io = io_new(&kFileCallbacks, mode, path, &it, sizeof(io_file_t));
    io->error = os_error(file);
    return io;
}
