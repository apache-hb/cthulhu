#pragma once

#include "io/io.h"

typedef struct fs_t fs_t;

fs_t *fs_physical(const char *root);
fs_t *fs_virtual(const char *name);

void fs_mkdir(fs_t *fs, const char *path);
io_t *fs_open(fs_t *fs, const char *path, file_flags_t flags);

void fs_rmdir(fs_t *fs, const char *path);
void fs_delete(fs_t *fs, const char *path);

void fs_copy(fs_t *fs, fs_t *dst);
