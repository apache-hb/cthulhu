// SPDX-License-Identifier: LGPL-3.0-only

#include "base/panic.h"
#include "io/impl.h"
#include "io/io.h"
#include "arena/arena.h"

#include "os/os.h"

typedef struct io_file_t
{
    os_file_t file;
    os_mapping_t mapping;
} io_file_t;

static io_file_t *fd_data(io_t *self)
{
    io_file_t *file = io_data(self);
    return file;
}

static size_t fd_read(io_t *self, void *dst, size_t size)
{
    io_file_t *file = fd_data(self);

    size_t read = 0;
    self->error = os_file_read(&file->file, dst, size, &read);
    if (self->error != 0) return SIZE_MAX;

    return read;
}

static size_t fd_write(io_t *self, const void *src, size_t size)
{
    io_file_t *file = fd_data(self);

    size_t written = 0;
    self->error = os_file_write(&file->file, src, size, &written);
    if (self->error != 0) return SIZE_MAX;

    return written;
}

static size_t fd_size(io_t *self)
{
    io_file_t *file = fd_data(self);

    size_t size = 0;
    self->error = os_file_size(&file->file, &size);
    if (self->error != 0) return SIZE_MAX;

    return size;
}

static size_t fd_seek(io_t *self, size_t offset)
{
    io_file_t *file = fd_data(self);

    size_t seek = 0;
    self->error = os_file_seek(&file->file, offset, &seek);
    if (self->error != 0) return SIZE_MAX;

    return seek;
}

static void *fd_map(io_t *self, os_protect_t protect)
{
    io_file_t *file = fd_data(self);

    if (!os_mapping_active(&file->mapping))
    {
        size_t size = io_size(self);
        self->error = os_file_map(&file->file, protect, size, &file->mapping);
        if (self->error != 0) return NULL;
    }

    return os_mapping_data(&file->mapping);
}

static void fd_close(io_t *self)
{
    io_file_t *file = fd_data(self);
    if (io_error(self) == 0)
    {
        os_file_unmap(&file->mapping);
        os_file_close(&file->file);
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
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    os_file_t fd = { 0 };
    os_error_t err = os_file_open(path, mode, &fd);

    io_file_t data = {
        .file = fd,
    };

    io_t *io = io_new(&kFileCallbacks, mode, path, &data, sizeof(io_file_t), arena);
    io->error = err;

    return io;
}
