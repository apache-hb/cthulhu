#include "compat.h"

#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>

FILE *compat_fopen(const char *path, const char *mode) {
    return fopen(path, mode);
}

bool compat_file_exists(const char *path) {
    return access(path, F_OK) != -1;
}
