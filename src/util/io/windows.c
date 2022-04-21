#include "common.h"

#include <windows.h>

typedef struct {
    HANDLE handle;
} windows_file_t;

static const file_ops_t OPS;

#define TOTAL_SIZE (sizeof(file_t) + sizeof(windows_file_t))

#define SELF(file) ((windows_file_t*)file->data)

static size_t windows_read(file_t *self, void *dst, size_t total) {

}

static size_t windows_write(file_t *self, const void *src, size_t total) {

}

static size_t windows_seek(file_t *self, size_t offset) {

}

static size_t windows_size(file_t *self) {

}

static size_t windows_tell(file_t *self) {

}

static const file_ops_t OPS = {
    .read = windows_read,
    .write = windows_write,
    .seek = windows_seek,
    .size = windows_size,
    .tell = windows_tell
};

static char *get_absolute(const char *path) {
    // the following is needed so we are aware of paths longer than MAX_PATH
    // GetFullPathNameA doesnt work on paths longer than MAX_PATH
    // but GetFullPathNameW does

    // convert our path to a wide string
    size_t len = strlen(path);
    int mbsize = MultiByteToWideChar(CP_UTF8, 0, path, len, NULL, 0);
    LPWSTR *total = ctu_malloc(sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, path, len, total, mbsize);

    // use the wide string to get the full path
    DWORD size = GetFullPathNameW(total, 0, NULL, NULL);
    LPWSTR result = ctu_malloc(size + 1);
    GetFullPathNameW(total, size, result, NULL);

    // turn the wide string back into a utf-8 string
    char *out = ctu_malloc(size + 1);
    wcstombs(out, result, size);

    // free the memory we dont need anymore
    ctu_free(result);
    ctu_free(total);

    return out;
}

void platform_close(file_t *file) {

}

void platform_open(file_t **file, const char *path, contents_t format, access_t access) {

}

bool file_exists(const char *path) {
    return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}
