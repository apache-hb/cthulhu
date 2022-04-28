#include "common.h"

static file_close_t kFileCloseCallbacks[] = {
    [FD] = platform_close,
    [MEMORY] = memory_close,
};

file_t *file_new(const char *path, contents_t format, access_t access) {
    file_t *self;

    platform_open(&self, path, format, access);

    return self;
}

file_t *
memory_new(const char *name, size_t size, contents_t format, access_t access) {
    file_t *self;

    memory_open(&self, name, size, format, access);

    return self;
}

void close_file(file_t *file) {
    if (file_ok(file)) {
        kFileCloseCallbacks[file->backing](file);
    }

    ctu_free(file);
}

size_t file_read(file_t *file, void *dst, size_t total) {
    return file->ops->read(file, dst, total);
}

size_t file_write(file_t *file, const void *src, size_t total) {
    return file->ops->write(file, src, total);
}

size_t file_seek(file_t *file, size_t offset) {
    return file->ops->seek(file, offset);
}

size_t file_size(file_t *file) {
    return file->ops->size(file);
}

size_t file_tell(file_t *file) {
    return file->ops->tell(file);
}

void *file_map(file_t *file) {
    return file->ops->mapped(file);
}

bool file_ok(file_t *file) {
    return file->ops->ok(file);
}
