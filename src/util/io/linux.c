#include "common.h"

#include "cthulhu/util/str.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

typedef struct {
    FILE *file;
} unix_file_t;

#define SELF(file) ((unix_file_t*)file->data)
#define TOTAL_SIZE (sizeof(file_t) + sizeof(unix_file_t))

static char *get_absolute(const char *path) {
    long size = pathconf(path, _PC_PATH_MAX);
    char *total = ctu_malloc(size + 1);

    if (getcwd(total, size) == NULL) {
        ctu_free(total);
        return NULL;
    }

    char *result = format("%s/%s", total, path);
    ctu_free(total);
    return result;
}

static size_t unix_read(file_t *self, void *dst, size_t total) {
    unix_file_t *file = SELF(self);
    return fread(dst, 1, total, file->file);
}

static size_t unix_write(file_t *self, const void *src, size_t total) {
    unix_file_t *file = SELF(self);
    return fwrite(src, 1, total, file->file);
}

static size_t unix_seek(file_t *self, size_t offset) {
    unix_file_t *file = SELF(self);
    return fseek(file->file, offset, SEEK_SET);
}

static size_t unix_size(file_t *self) {
    unix_file_t *file = SELF(self);
    struct stat st;

    if (fstat(fileno(file->file), &st) == -1) {
        return SIZE_MAX;
    }

    return st.st_size;
}

static size_t unix_tell(file_t *self) {
    unix_file_t *file = SELF(self);
    return ftell(file->file);
}

static void *unix_map(file_t *self) {
    unix_file_t *file = SELF(self);
    int fd = fileno(file->file);
    int prot = self->access == READ ? PROT_READ : PROT_READ | PROT_WRITE;
    size_t len = unix_size(self);
    void *ptr = mmap(NULL, len, prot, MAP_PRIVATE, fd, 0);
    
    return ptr == MAP_FAILED ? NULL : ptr;
}

static bool unix_ok(file_t *self) {
    unix_file_t *file = SELF(self);
    return file->file != NULL;
}

static const file_ops_t kFileOps = {
    .read = unix_read,
    .write = unix_write,
    .seek = unix_seek,
    .size = unix_size,
    .tell = unix_tell,
    .mapped = unix_map,
    .ok = unix_ok
};

void platform_close(file_t *file) {
    unix_file_t *self = SELF(file);
    fclose(self->file);
}

void platform_open(file_t **file, const char *path, contents_t format, access_t access) {
    file_t *self = ctu_malloc(TOTAL_SIZE);
    self->path = get_absolute(path);
    self->format = format;
    self->access = access;
    self->backing = FD;
    self->ops = &kFileOps;
    
    char mode[] = {
        access == WRITE ? 'w' : 'r',
        format == BINARY ? 'b' : '\0',
        '\0'
    };
    unix_file_t *nix = SELF(self);
    nix->file = fopen(path, mode);

    *file = self;
}

bool file_exists(const char *path) {
    return access(path, F_OK) != -1;
}
