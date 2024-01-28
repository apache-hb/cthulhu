#include "io/impl.h"

#include "core/macros.h"
#include "arena/arena.h"

#include "base/panic.h"
#include "base/util.h"

/// @brief a read/write in memory file
typedef struct buffer_t
{
    char *data;   ///< stored data
    size_t used;  ///< used data
    size_t total; ///< total size of data

    size_t offset; ///< current offset in data
} buffer_t;

static buffer_t *mem_data(io_t *self)
{
    return io_data(self);
}

static size_t mem_read(io_t *self, void *dst, size_t size)
{
    buffer_t *mem = mem_data(self);
    size_t len = MIN(size, mem->used - mem->offset);
    ctu_memcpy(dst, mem->data + mem->offset, len);
    mem->offset += len;
    return len;
}

static size_t mem_write(io_t *self, const void *src, size_t size)
{
    buffer_t *mem = mem_data(self);
    mem->used = MAX(mem->used, mem->offset + size);
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
    buffer_t *mem = mem_data(self);
    return mem->used;
}

static size_t mem_seek(io_t *self, size_t offset)
{
    buffer_t *mem = mem_data(self);
    mem->offset = MIN(offset, mem->used);
    return mem->offset;
}

static void *mem_map(io_t *self, os_protect_t protect)
{
    CTASSERTF(!(protect & eProtectExecute), "cannot map memory object as executable `%s`", io_name(self));

    buffer_t *mem = mem_data(self);

    return mem->data;
}

static void mem_close(io_t *self)
{
    buffer_t *mem = mem_data(self);
    arena_free(mem->data, mem->total, self->arena);
}

static const io_callbacks_t kBufferCallbacks = {
    .fn_read = mem_read,
    .fn_write = mem_write,

    .fn_get_size = mem_size,
    .fn_seek = mem_seek,

    .fn_map = mem_map,
    .fn_close = mem_close
};

USE_DECL
io_t *io_memory(const char *name, const void *data, size_t size, os_access_t flags, arena_t *arena)
{
    CTASSERT(name != NULL);
    CTASSERT(data != NULL);
    CTASSERT(arena != NULL);

    buffer_t buffer = {
        .data = ARENA_MALLOC(size, "memory", NULL, arena),
        .total = size,
        .used = size,
        .offset = 0
    };

    ctu_memcpy(buffer.data, data, size);

    io_t *io = io_new(&kBufferCallbacks, flags, name, &buffer, sizeof(buffer_t), arena);
    ARENA_REPARENT(buffer.data, io, arena);
    return io;
}

USE_DECL
io_t *io_blob(const char *name, size_t size, os_access_t flags, arena_t *arena)
{
    CTASSERT(name != NULL);
    CTASSERT(arena != NULL);

    buffer_t buffer = {
        .data = ARENA_MALLOC(size, "blob", NULL, arena),
        .total = size,
        .used = 0,
        .offset = 0
    };

    io_t *io = io_new(&kBufferCallbacks, flags, name, &buffer, sizeof(buffer_t), arena);
    ARENA_REPARENT(buffer.data, io, arena);
    return io;
}
