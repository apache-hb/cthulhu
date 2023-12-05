#include "io/impl.h"

#include "core/macros.h"

#include <string.h>

/// @brief a non-owning, read only view of data
typedef struct view_t
{
    const char *data;   ///< pointer to data
    size_t size;        ///< size of data
    size_t offset;      ///< current offset in data
} view_t;

static view_t *view_data(io_t *self)
{
    return io_data(self);
}

static size_t view_read(io_t *self, void *dst, size_t size)
{
    view_t *mem = view_data(self);
    size_t len = MIN(size, mem->size - mem->offset);
    memcpy(dst, mem->data + mem->offset, len);
    mem->offset += len;
    return len;
}

static size_t view_size(io_t *self)
{
    view_t *mem = view_data(self);
    return mem->size;
}

static size_t view_seek(io_t *self, size_t offset)
{
    view_t *mem = view_data(self);
    mem->offset = MIN(offset, mem->size);
    return mem->offset;
}

static const void *view_map(io_t *self)
{
    view_t *mem = view_data(self);

    return mem->data;
}

static const io_callbacks_t kViewCallbacks = {
    .fn_read = view_read,
    .fn_write = NULL,

    .fn_get_size = view_size,
    .fn_seek = view_seek,

    .fn_map = view_map
};

USE_DECL
io_t *io_view(const char *name, const void *data, size_t size)
{
    os_access_t flags = eAccessRead;

    view_t view = {
        .data = data,
        .size = size,
        .offset = 0
    };

    return io_new(&kViewCallbacks, flags, name, &view, sizeof(view_t));
}

USE_DECL
io_t *io_string(const char *name, const char *string)
{
    return io_view(name, string, strlen(string));
}
