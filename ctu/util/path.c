#include "io.h"
#include "str.h"
#include "util.h"

#include <unistd.h>
#include <string.h>

static const char *path_ext(const char *path) {
    const char *ext = strrchr(path, '.');
    if (ext == NULL) {
        return "";
    }
    return ext + 1;
}

path_t *new_path(const char *base, const char *relative, const char *ext) {
    path_t *path = ctu_malloc(sizeof(path_t));
    path->base = base;
    path->relative = relative;
    path->ext = ext;
    return path;
}

path_t *root_path(void) {
    char *cwd = getcwd(ctu_malloc(PATH_MAX + 1), PATH_MAX + 1);
    return new_path(cwd, "", "");
}

path_t *relative_path(path_t *base, const char *path) {
    const char *ext = path_ext(path);
    size_t trim = strlen(path) - strlen(ext) - (strlen(ext) > 0 ? 1 : 0);
    const char *relative = ctu_strndup(path, trim);
    return new_path(base->base, relative, ext);
}

char *path_realpath(const path_t *path) {
    return format("%s%s%s.%s", path->base, PATH_SEP, path->relative, path->ext);
}

char *path_relative(const path_t *path) {
    return format("%s.%s", path->relative, path->ext);   
}

char *path_noext(const path_t *path) {
    return format("%s%s%s", path->base, PATH_SEP, path->relative);
}

bool path_exists(const path_t *path) {
    return access(path_realpath(path), F_OK) != -1;
}

static const char *get_mode(file_mode_t mode) {
    switch (mode) {
    case FILE_READ: return "rb";
    case FILE_WRITE: return "w";
    default: return "rb";
    }
}

file_t *path_open(path_t *path, file_mode_t mode) {
    printf("open: %s\n", path_realpath(path));
    FILE *ptr = fopen(path_realpath(path), get_mode(mode));
    if (ptr == NULL) {
        return NULL;
    }

    file_t *file = ctu_malloc(sizeof(file_t));
    file->path = path;
    file->file = ptr;
    return file;
}

vector_t *path_parts(const path_t *path) {
    return strsplit(path_noext(path), PATH_SEP);
}

size_t file_size(file_t *fp) {
    fseek(fp->file, 0, SEEK_END);
    size_t size = ftell(fp->file);
    fseek(fp->file, 0, SEEK_SET);
    return size;
}

void *file_map(file_t *fp) {
    int fd = fileno(fp->file);
    void *data = mmap(NULL, file_size(fp), PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        return NULL;
    }
    return data;
}
