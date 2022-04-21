#include "common.h"

#include <string.h>

typedef struct {
    char *data;
    size_t size;
    size_t used;
    size_t cursor;
} memory_file_t;

#define TOTAL_SIZE (sizeof(ctu_file_t) + sizeof(memory_file_t))
#define SELF(file) ((memory_file_t*)file->data)

static size_t mem_read(ctu_file_t *self, void *dst, size_t total) {
    memory_file_t *file = SELF(self);
    size_t read = MIN(total, file->used - file->cursor);
    memcpy(dst, file->data + file->cursor, read);
    file->cursor += read;
    return read;
}

static size_t mem_write(ctu_file_t *self, const void *src, size_t total) {
    
}

static size_t mem_seek(ctu_file_t *self, size_t offset) {
    memory_file_t *file = SELF(self);
    size_t cursor = MIN(offset, file->used);
    file->cursor = cursor;
    return cursor;
}

static size_t mem_size(ctu_file_t *self) {
    memory_file_t *file = SELF(self);
    return file->used;
}

static size_t mem_tell(ctu_file_t *self) {
    memory_file_t *file = SELF(self);
    return file->cursor;
}

static void *mem_map(ctu_file_t *self) {
    memory_file_t *file = SELF(self);
    return file->data;
}

static bool mem_ok(ctu_file_t *self) {
    UNUSED(self);
    return true;
}

static file_ops_t OPS = { 
    .read = mem_read,
    .write = mem_write,
    .seek = mem_seek,
    .size = mem_size,
    .tell = mem_tell,
    .mapped = mem_map,
    .ok = mem_ok
};

void memory_close(ctu_file_t *file) {

}

void memory_open(ctu_file_t **file, const char *name, size_t size, contents_t format, access_t access) {
    ctu_file_t *self = ctu_malloc(TOTAL_SIZE);
    self->path = name;
    self->format = format;
    self->access = access;
    self->backing = MEMORY;
    self->ops = &OPS;

    *file = self;
}
