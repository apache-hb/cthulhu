#include "io/io.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

STATIC_ASSERT(sizeof(io_error_t) == sizeof(cerror_t), "io error and cerror must be the same size");

typedef size_t (*io_read_t)(io_t *self, void *dst, size_t size);
typedef size_t (*io_write_t)(io_t *self, const void *src, size_t size);

typedef size_t (*io_size_t)(io_t *self);

typedef const void *(*io_map_t)(io_t *self);

typedef void (*io_close_t)(io_t *self);

typedef struct io_callbacks_t
{
    io_read_t read;
    io_write_t write;

    io_size_t size;

    io_map_t map;

    io_close_t close;
} io_callbacks_t;

typedef struct io_t
{
    alloc_t *alloc;
    const io_callbacks_t *cb;

    io_error_t error;
    file_flags_t flags;
    const char *name;

    size_t dataSize;
    char data[];
} io_t;

typedef struct buffer_t
{
    char *data;   ///< stored data
    size_t used;  ///< used data
    size_t total; ///< total size of data

    size_t offset; ///< current offset in data
} buffer_t;

typedef struct view_t
{
    const char *data;
    size_t size;
    size_t offset;
} view_t;

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

static void fd_close(io_t *self)
{
    file_t *file = io_data(self);
    file_close(*file);
}

/// memory callbacks

static size_t mem_read(io_t *self, void *dst, size_t size)
{
    buffer_t *mem = io_data(self);
    size_t len = MIN(size, mem->used - mem->offset);
    memcpy(dst, mem->data + mem->offset, len);
    mem->offset += len;
    return len;
}

static size_t mem_write(io_t *self, const void *src, size_t size)
{
    buffer_t *mem = io_data(self);
    if (mem->offset + size > mem->total)
    {
        mem->data = arena_realloc(self->alloc, mem->data, mem->offset + size, mem->total);
        mem->total = mem->offset + size;
        mem->used = mem->offset + size;
    }

    memcpy(mem->data + mem->offset, src, size);
    mem->offset += size;

    return size;
}

static size_t mem_size(io_t *self)
{
    buffer_t *mem = io_data(self);
    return mem->used;
}

static const void *mem_map(io_t *self)
{
    buffer_t *mem = io_data(self);

    void *it = arena_malloc(self->alloc, mem->used, "mem-map");
    memcpy(it, mem->data, mem->used);

    return it;
}

static void mem_close(io_t *self)
{
    buffer_t *mem = io_data(self);
    arena_free(self->alloc, mem->data, mem->total);
}

static size_t view_read(io_t *self, void *dst, size_t size)
{
    view_t *mem = io_data(self);
    size_t len = MIN(size, mem->size - mem->offset);
    memcpy(dst, mem->data + mem->offset, len);
    mem->offset += len;
    return len;
}

static size_t view_size(io_t *self)
{
    view_t *mem = io_data(self);
    return mem->size;
}

static const void *view_map(io_t *self)
{
    view_t *mem = io_data(self);

    return mem->data;
}

/// view callbacks

static const io_callbacks_t kFileCallbacks = {.read = fd_read,
                                              .write = fd_write,

                                              .size = fd_size,

                                              .map = fd_map,
                                              .close = fd_close};

static const io_callbacks_t kBufferCallbacks = {.read = mem_read,
                                                .write = mem_write,

                                                .size = mem_size,

                                                .map = mem_map,
                                                .close = mem_close};

static const io_callbacks_t kViewCallbacks = {.read = view_read,
                                              .write = NULL,

                                              .size = view_size,

                                              .map = view_map};

static io_t *io_new(alloc_t *alloc, const io_callbacks_t *cb, file_flags_t flags, const char *name, void *data,
                    size_t size)
{
    io_t *io = arena_malloc(alloc, sizeof(io_t) + size, name);

    memcpy(io->data, data, size);
    io->alloc = alloc;
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
    io_t *io = io_new(alloc, &kFileCallbacks, mode, path, &file, sizeof(file_t));
    io->error = err;
    return io;
}

io_t *io_memory(alloc_t *alloc, const char *name, const void *data, size_t size)
{
    file_flags_t flags = eFileRead | eFileWrite;

    buffer_t buffer = {.data = arena_malloc(alloc, size, "io-memory"), .total = size, .used = size, .offset = 0};

    if (data != NULL)
    {
        memcpy(buffer.data, data, size);
    }

    return io_new(alloc, &kBufferCallbacks, flags, name, &buffer, sizeof(buffer_t));
}

io_t *io_view(alloc_t *alloc, const char *name, const void *data, size_t size)
{
    file_flags_t flags = eFileRead;

    view_t view = {.data = data, .size = size, .offset = 0};

    return io_new(alloc, &kViewCallbacks, flags, name, &view, sizeof(view_t));
}

io_t *io_string(alloc_t *alloc, const char *name, const char *string)
{
    return io_view(alloc, name, string, strlen(string));
}

void io_close(io_t *io)
{
    CTASSERT(io != NULL);

    if (io->cb->close != NULL)
    {
        io->cb->close(io);
    }

    arena_free(io->alloc, io, sizeof(io_t) + io->dataSize);
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

size_t io_size(io_t *io)
{
    CTASSERT(io != NULL);

    return io->cb->size(io);
}

const char *io_name(io_t *io)
{
    CTASSERT(io != NULL);

    return io->name;
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
