// SPDX-License-Identifier: LGPL-3.0-only

#include "arena/arena.h"
#include "io/impl.h"
#include "io/impl/view.h"

#include "base/util.h"
#include "base/panic.h"

#include "core/macros.h"

static io_view_impl_t *view_data(io_t *self)
{
    return io_data(self);
}

static size_t view_read(io_t *self, void *dst, size_t size)
{
    io_view_impl_t *mem = view_data(self);
    size_t len = CT_MIN(size, mem->size - mem->offset);
    ctu_memcpy(dst, mem->data + mem->offset, len);
    mem->offset += len;
    return len;
}

static size_t view_size(io_t *self)
{
    io_view_impl_t *mem = view_data(self);
    return mem->size;
}

static size_t view_seek(io_t *self, size_t offset)
{
    io_view_impl_t *mem = view_data(self);
    mem->offset = CT_MIN(offset, mem->size);
    return mem->offset;
}

static void *view_map(io_t *self, os_protect_t protect)
{
    CTASSERTF(protect == eOsProtectRead, "cannot map view with protection %d", protect);
    io_view_impl_t *mem = view_data(self);

    return (void*)mem->data;
}

static const io_callbacks_t kViewCallbacks = {
    .fn_read = view_read,

    .fn_get_size = view_size,
    .fn_seek = view_seek,

    .fn_map = view_map,

    .size = sizeof(io_view_impl_t),
};

static io_t *impl_view_init(void *buffer, const char *name, const void *data, size_t size, arena_t *arena)
{
    CTASSERT(data != NULL);

    io_view_impl_t impl = {
        .data = data,
        .size = size,
        .offset = 0
    };

    return io_init(buffer, &kViewCallbacks, eOsAccessRead, name, &impl, arena);
}

static io_t *impl_string_init(void *buffer, const char *name, const char *string, arena_t *arena)
{
    CTASSERT(string != NULL);

    return impl_view_init(buffer, name, string, ctu_strlen(string), arena);
}

///
/// public allocating api
///

STA_DECL
io_t *io_view(const char *name, const void *data, size_t size, arena_t *arena)
{
    void *buffer = ARENA_MALLOC(IO_VIEW_SIZE, name, NULL, arena);
    return impl_view_init(buffer, name, data, size, arena);
}

STA_DECL
io_t *io_string(const char *name, const char *string, arena_t *arena)
{
    void *buffer = ARENA_MALLOC(IO_VIEW_SIZE, name, NULL, arena);
    return impl_string_init(buffer, name, string, arena);
}

///
/// public in place api
///

STA_DECL
io_t *io_view_init(void *buffer, const char *name, const void *data, size_t size)
{
    return impl_view_init(buffer, name, data, size, NULL);
}

STA_DECL
io_t *io_string_init(void *buffer, const char *name, const char *string)
{
    return impl_string_init(buffer, name, string, NULL);
}
