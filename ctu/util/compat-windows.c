#include "compat.h"
#include "util.h"

#include <stdio.h>
#include <windows.h>

FILE *compat_fopen(const char *path, const char *mode) {
    FILE *file;
    errno_t err = fopen_s(&file, path, mode);
    if (err != 0) {
        return NULL;
    }
    return file;
}

bool compat_file_exists(const char *path) {
    return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}

char *compat_realpath(const char *path) {
    char *result = ctu_malloc(MAX_PATH + 1);
    GetFullPathNameA(path, MAX_PATH + 1, result, NULL);
    return result;
}