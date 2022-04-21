#include "common.h"

static file_close_t CLOSES[] = {
    [FD] = platform_close,
    [MEMORY] = memory_close
};

ctu_file_t *file_new(const char *path, contents_t format, access_t access) {
    ctu_file_t *self;

    platform_open(&self, path, format, access);

    return self;
}

ctu_file_t *memory_new(const char *name, size_t size, contents_t format, access_t access) {
    ctu_file_t *self;

    memory_open(&self, name, size, format, access);

    return self;
}

void file_close(ctu_file_t *file) {
    CLOSES[file->backing](file);
    ctu_free(file);
}

size_t file_read(ctu_file_t *file, void *dst, size_t total) {
    return file->ops->read(file, dst, total);
}

size_t file_write(ctu_file_t *file, const void *src, size_t total) {
    return file->ops->write(file, src, total);
}

size_t file_seek(ctu_file_t *file, size_t offset) {
    return file->ops->seek(file, offset);
}

size_t _file_size(ctu_file_t *file) {
    return file->ops->size(file);
}

size_t file_tell(ctu_file_t *file) {
    return file->ops->tell(file);
}

void *file_map(ctu_file_t *file) {
    return file->ops->mapped(file);
}

bool file_ok(ctu_file_t *file) {
    return file->ops->ok(file);
}
