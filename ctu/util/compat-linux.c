#include "compat.h"

FILE *compat_fopen(const char *path, const char *mode) {
    return fopen(path, mode);
}

bool compat_file_exists(const char *path) {
    return access(path, F_OK) != -1;
}

char *compat_realpath(const char *path) {
    return realpath(path, NULL);
}
