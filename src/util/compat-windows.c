#include "compat.h"
#include "cthulhu/util/util.h"

// god would have given us a better way to do this
#pragma warning(push, 0)

#include <stdio.h>
#include <windows.h>

#pragma warning(pop)

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

char *ctu_realpath(const char *path) {
    char full[MAX_PATH];

    GetFullPathName(path, MAX_PATH, full, NULL);

    return ctu_strdup(full);
}
