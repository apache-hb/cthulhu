#pragma once

#include "ctu/util/io.h"

typedef struct {
    FILE *handle;
    const char *path;
} file_t;

void init_fs(void);
