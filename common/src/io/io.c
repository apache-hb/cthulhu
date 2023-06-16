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
    io_read_t fnRead;
    io_write_t fnWrite;

    io_size_t fnGetSize;

    io_map_t fnMap;

    io_close_t fnClose;
} io_callbacks_t;

typedef struct io_t
{
    const io_callbacks_t *cb;

    io_error_t error;
    file_flags_t flags;
    const char *name;
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
        mem->data = ctu_realloc(mem->data, mem->offset + size);
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

    void *it = ctu_malloc(mem->used);
    memcpy(it, mem->data, mem->used);

    return it;
}

static void mem_close(io_t *self)
{
    buffer_t *mem = io_data(self);
    ctu_free(mem->data);
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

static const io_callbacks_t kFileCallbacks = {
    .fnRead = fd_read,
    .fnWrite = fd_write,

    .fnGetSize = fd_size,

    .fnMap = fd_map,
    .fnClose = fd_close
};

static const io_callbacks_t kBufferCallbacks = {
    .fnRead = mem_read,
    .fnWrite = mem_write,

    .fnGetSize = mem_size,

    .fnMap = mem_map,
    .fnClose = mem_close
};

static const io_callbacks_t kViewCallbacks = {
    .fnRead = view_read,
    .fnWrite = NULL,

    .fnGetSize = view_size,

    .fnMap = view_map
};

static io_t *io_new(const io_callbacks_t *cb, file_flags_t flags, const char *name, void *data,
                    size_t size)
{
    io_t *io = ctu_malloc(sizeof(io_t) + size);

    memcpy(io->data, data, size);
    io->cb = cb;
    io->name = name;
    io->flags = flags;
    io->error = 0;

    return io;
}

USE_DECL
io_t *io_file(const char *path, file_flags_t mode)
{
    cerror_t err = 0;
    file_t file = file_open(path, mode, &err);
    io_t *io = io_new(&kFileCallbacks, mode, path, &file, sizeof(file_t));
    io->error = err;
    return io;
}

USE_DECL
io_t *io_memory(const char *name, const void *data, size_t size)
{
    file_flags_t flags = eFileRead | eFileWrite;

    buffer_t buffer = {.data = ctu_malloc(size), .total = size, .used = size, .offset = 0};

    if (data != NULL)
    {
        memcpy(buffer.data, data, size);
    }

    return io_new(&kBufferCallbacks, flags, name, &buffer, sizeof(buffer_t));
}

USE_DECL
io_t *io_blob(const char *name, size_t size)
{
    buffer_t buffer = {
        .data = ctu_malloc(size),
        .total = size,
        .used = size,
        .offset = 0
    };

    return io_new(&kBufferCallbacks, eFileWrite, name, &buffer, sizeof(buffer_t));
}

USE_DECL
io_t *io_view(const char *name, const void *data, size_t size)
{
    file_flags_t flags = eFileRead;

    view_t view = {.data = data, .size = size, .offset = 0};

    return io_new(&kViewCallbacks, flags, name, &view, sizeof(view_t));
}

USE_DECL
io_t *io_string(const char *name, const char *string)
{
    return io_view(name, string, strlen(string));
}

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
