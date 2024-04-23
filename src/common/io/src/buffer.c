// SPDX-License-Identifier: LGPL-3.0-only

#include "io/impl.h"
#include "io/impl/buffer.h"
#include "os/os.h"

#include "core/macros.h"
#include "arena/arena.h"

#include "base/panic.h"
#include "base/util.h"

static io_buffer_impl_t *mem_data(io_t *self)
{
    return io_data(self);
}

static size_t mem_read(io_t *self, void *dst, size_t size)
{
    io_buffer_impl_t *mem = mem_data(self);
    size_t len = CT_MIN(size, mem->used - mem->offset);
    ctu_memcpy(dst, mem->data + mem->offset, len);
    mem->offset += len;
    return len;
}

static size_t mem_write(io_t *self, const void *src, size_t size)
{
    io_buffer_impl_t *mem = mem_data(self);
    mem->used = CT_MAX(mem->used, mem->offset + size);
    if (mem->offset + size > mem->total)
    {
        mem->data = arena_realloc(mem->data, mem->offset + size, mem->total, self->arena);
        mem->total = mem->offset + size;
    }

    ctu_memcpy(mem->data + mem->offset, src, size);
    mem->offset += size;

    return size;
}

static size_t mem_size(io_t *self)
{
    io_buffer_impl_t *mem = mem_data(self);
    return mem->used;
}

static size_t mem_seek(io_t *self, size_t offset)
{
    io_buffer_impl_t *mem = mem_data(self);
    mem->offset = CT_MIN(offset, mem->used);
    return mem->offset;
}

static void *mem_map(io_t *self, os_protect_t protect)
{
    CTASSERTF(!(protect & eOsProtectExecute), "cannot map memory object as executable `%s`", io_name(self));

    io_buffer_impl_t *mem = mem_data(self);

    return mem->data;
}

static os_error_t mem_close(io_t *self)
{
    io_buffer_impl_t *mem = mem_data(self);
    arena_free(mem->data, mem->total, self->arena);

    return eOsSuccess;
}

static const io_callbacks_t kBufferCallbacks = {
    .fn_read = mem_read,
    .fn_write = mem_write,

    .fn_get_size = mem_size,
    .fn_seek = mem_seek,

    .fn_map = mem_map,
    .fn_close = mem_close,

    .size = sizeof(io_buffer_impl_t)
};

static io_t *impl_memory_init(void *buffer, const char *name, const void *data, size_t size, os_access_t flags, arena_t *arena)
{
    CTASSERT(data != NULL);

    io_buffer_impl_t impl = {
        .data = ARENA_MALLOC(size, "memory", NULL, arena),
        .total = size,
        .used = size,
        .offset = 0
    };

    ctu_memcpy(impl.data, data, size);

    return io_init(buffer, &kBufferCallbacks, flags, name, &impl, arena);
}

static io_t *impl_blob_init(void *buffer, const char *name, size_t size, os_access_t flags, arena_t *arena)
{
    io_buffer_impl_t impl = {
        .data = ARENA_MALLOC(size, "blob", NULL, arena),
        .total = size,
        .used = 0,
        .offset = 0
    };

    return io_init(buffer, &kBufferCallbacks, flags, name, &impl, arena);
}

///
/// public allocating api
///

STA_DECL
io_t *io_memory(const char *name, const void *data, size_t size, os_access_t flags, arena_t *arena)
{
    void *buffer = ARENA_MALLOC(IO_BUFFER_SIZE, name, NULL, arena);
    return impl_memory_init(buffer, name, data, size, flags, arena);
}

STA_DECL
io_t *io_blob(const char *name, size_t size, os_access_t flags, arena_t *arena)
{
    void *buffer = ARENA_MALLOC(IO_BUFFER_SIZE, name, NULL, arena);
    return impl_blob_init(buffer, name, size, flags, arena);
}

///
/// public in place api
///

STA_DECL
io_t *io_memory_init(void *buffer, const char *name, const void *data, size_t size, os_access_t flags, arena_t *arena)
{
    return impl_memory_init(buffer, name, data, size, flags, arena);
}

STA_DECL
io_t *io_blob_init(void *buffer, const char *name, size_t size, os_access_t flags, arena_t *arena)
{
    return impl_blob_init(buffer, name, size, flags, arena);
}
