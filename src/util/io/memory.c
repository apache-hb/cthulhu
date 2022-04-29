#include "common.h"

#include <string.h>

typedef struct
{
    char *data;
    size_t size;
    size_t used;
    size_t cursor;
} memory_file_t;

#define TOTAL_SIZE (sizeof(file_t) + sizeof(memory_file_t))
#define SELF(file) ((memory_file_t *)(file)->data)

static size_t mem_read(file_t *self, void *dst, size_t total)
{
    memory_file_t *file = SELF(self);
    size_t read = MIN(total, file->used - file->cursor);
    memcpy(dst, file->data + file->cursor, read);
    file->cursor += read;
    return read;
}

static size_t mem_write(file_t *self, const void *src, size_t total)
{
    memory_file_t *file = SELF(self);
    if (file->cursor + total > file->size)
    {
        size_t resize = MAX(file->size * 2, file->cursor + total);
        file->data = ctu_realloc(file->data, resize);
        file->size = resize;
    }

    memcpy(file->data + file->cursor, src, total);
    file->cursor += total;
    file->used = MAX(file->used, file->cursor);
    return total;
}

static size_t mem_seek(file_t *self, size_t offset)
{
    memory_file_t *file = SELF(self);
    size_t cursor = MIN(offset, file->used);
    file->cursor = cursor;
    return cursor;
}

static size_t mem_size(file_t *self)
{
    memory_file_t *file = SELF(self);
    return file->used;
}

static size_t mem_tell(file_t *self)
{
    memory_file_t *file = SELF(self);
    return file->cursor;
}

static void *mem_map(file_t *self)
{
    memory_file_t *file = SELF(self);
    return file->data;
}

static bool mem_ok(file_t *self)
{
    UNUSED(self);
    return true;
}

static file_ops_t kMemoryOps = {
    .read = mem_read,
    .write = mem_write,
    .seek = mem_seek,
    .size = mem_size,
    .tell = mem_tell,
    .mapped = mem_map,
    .ok = mem_ok,
};

void memory_close(file_t *file)
{
    ctu_free(SELF(file)->data);
}

void memory_open(file_t **file, const char *name, size_t size, contents_t format, access_t access)
{
    file_t *self = ctu_malloc(TOTAL_SIZE);
    self->path = name;
    self->format = format;
    self->access = access;
    self->backing = MEMORY;
    self->ops = &kMemoryOps;

    memory_file_t *mem = SELF(self);
    mem->data = ctu_malloc(size);
    mem->size = size;
    mem->used = 0;
    mem->cursor = 0;

    *file = self;
}
