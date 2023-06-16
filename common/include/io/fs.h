#pragma once

#include "io/io.h"

typedef struct fs_t fs_t;

fs_t *fs_physical(const char *root);
fs_t *fs_virtual(const char *name);

void fs_mkdir(fs_t *fs, const char *path);
