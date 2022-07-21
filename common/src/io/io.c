#include "io/io.h"

#include "base/panic.h"
#include "base/memory.h"

#include <string.h>

STATIC_ASSERT(sizeof(io_error_t) == sizeof(cerror_t), "io error and cerror must be the same size");

typedef size_t(*io_read_t)(io_t *self, void *dst, size_t size);
typedef size_t(*io_write_t)(io_t *self, const void *src, size_t size);

typedef size_t(*io_tell_t)(io_t *self);
typedef size_t(*io_seek_t)(io_t *self, size_t offset);
typedef size_t(*io_size_t)(io_t *self);

typedef const void*(*io_map_t)(io_t *self);

typedef struct io_callbacks_t {
    io_read_t read;
    io_write_t write;

    io_tell_t tell;
    io_seek_t seek;
    io_size_t size;

    io_map_t map;
} io_callbacks_t;

typedef struct io_t {
    const io_callbacks_t *cb;

    io_error_t error;
    file_flags_t flags;
    const char *name;

    size_t dataSize;
    char data[];
} io_t;

typedef struct buffer_t {
    void *data; ///< stored data
    size_t used; ///< used data
    size_t total; ///< total size of data

    size_t offset; ///< current offset in data
} buffer_t;

static void *io_data(io_t *io)
{
    return io->data;
}

/// file callbacks

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

typedef size_t(*io_tell_t)(io_t *self);
typedef size_t(*io_seek_t)(io_t *self, size_t offset);

static size_t fd_size(io_t *self)
{
    file_t *file = io_data(self);
    return file_size(*file, &self->error);
}

static const void *fd_map(io_t *self)
{
    file_t *file = io_data(self);
    return file_map(*file, &self->error);
}

/// memory callbacks

typedef size_t(*io_read_t)(io_t *self, void *dst, size_t size);
typedef size_t(*io_write_t)(io_t *self, const void *src, size_t size);

typedef size_t(*io_offset_t)(io_t *self);
typedef size_t(*io_seek_t)(io_t *self, size_t offset);
typedef size_t(*io_size_t)(io_t *self);

static const io_callbacks_t kFileCallbacks = {
    .read = fd_read,
    .write = fd_write,
    .seek = NULL,
    .tell = NULL,
    .size = fd_size,
    .map = fd_map
};

static const io_callbacks_t kBufferCallbacks = {};

static io_t *io_new(alloc_t *alloc, const io_callbacks_t *cb, file_flags_t flags, const char *name, void *data, size_t size)
{
    io_t *io = arena_malloc(alloc, sizeof(io_t) + size, name);

    memcpy(io->data, data, size);
    io->cb = cb;
    io->dataSize = size;
    io->name = name;
    io->flags = flags;
    io->error = 0;

    return io;
}

io_t *io_file(alloc_t *alloc, const char *path, file_flags_t mode)
{
    cerror_t err = 0;
    file_t file = file_open(path, mode, &err);
    return io_new(alloc, &kFileCallbacks, mode, path, &file, sizeof(file_t));
}

io_t *io_memory(alloc_t *alloc, const char *name, const void *data, size_t size)
{
    file_flags_t flags = eFileRead;
    if (data == NULL)
    {
        flags |= eFileWrite;
    }

    buffer_t buffer = {
        .data = (void*)data,
        .total = size,
        .used = size,
        .offset = 0
    };

    return io_new(alloc, &kBufferCallbacks, flags, name, &buffer, size);
}

size_t io_read(io_t *io, void *dst, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERT(io->flags & eFileRead);

    return io->cb->read(io, dst, size);
}

size_t io_write(io_t *io, const void *src, size_t size)
{
    CTASSERT(io != NULL);
    CTASSERT(io->flags & eFileWrite);

    return io->cb->write(io, src, size);
}

size_t io_tell(io_t *io)
{
    CTASSERT(io != NULL);

    return io->cb->tell(io);
}

size_t io_seek(io_t *io, size_t offset)
{
    CTASSERT(io != NULL);

    return io->cb->seek(io, offset);
}

size_t io_size(io_t *io)
{
    CTASSERT(io != NULL);

    return io->cb->size(io);
}

const void *io_map(io_t *io)
{
    CTASSERT(io != NULL);

    return io->cb->map(io);
}

io_error_t io_error(const io_t *io)
{
    CTASSERT(io != NULL);

    return io->error;
}
