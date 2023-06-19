#pragma once

#include "io/io.h"

BEGIN_API

typedef struct fs_t fs_t;

fs_t *fs_physical(const char *root);
fs_t *fs_virtual(const char *name);

void fs_mkdir(fs_t *fs, const char *path);
io_t *fs_open(fs_t *fs, const char *path, file_flags_t flags);

void fs_rmdir(fs_t *fs, const char *path);
void fs_delete(fs_t *fs, const char *path);

void fs_copy(fs_t *fs, fs_t *dst);

// v2

typedef struct fs2_t fs2_t;
typedef struct fs2_dir_t fs2_dir_t;
typedef struct fs2_file_t fs2_file_t;

fs2_t *fs2_physical(const char *root);
fs2_t *fs2_virtual(const char *name);

fs2_dir_t *fs2_dir_create(fs2_t *fs, const char *path);
fs2_file_t *fs2_file_create(fs2_t *fs, const char *path, file_flags_t flags);

void fs2_dir_delete(fs2_dir_t *dir);
void fs2_file_delete(fs2_file_t *file);

void fs2_sync(fs2_t *dst, fs2_t *src);

END_API
