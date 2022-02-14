#include "compat.h"

#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <limits.h>

#include "cthulhu/util/str.h"

FILE *compat_fopen(const char *path, const char *mode) {
    return fopen(path, mode);
}

bool compat_file_exists(const char *path) {
    return access(path, F_OK) != -1;
}

const char *compat_realpath(const char *path) {
    char full[PATH_MAX + 1];
    realpath(path, full);
    return ctu_strdup(full);
}
