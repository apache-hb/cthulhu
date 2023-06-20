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

typedef struct reports_t reports_t;

typedef struct fs2_t fs2_t;

fs2_t *fs2_physical(reports_t *reports, const char *root);
fs2_t *fs2_virtual(reports_t *reports, const char *name);

void fs2_dir_create(fs2_t *fs, const char *path);
void fs2_file_create(fs2_t *fs, const char *path, file_flags_t flags);

io_t *fs2_open(fs2_t *fs, const char *path, file_flags_t flags);

bool fs2_dir_exists(fs2_t *fs, const char *path);
bool fs2_file_exists(fs2_t *fs, const char *path);

void fs2_dir_delete(fs2_t *fs, const char *path);
void fs2_file_delete(fs2_t *fs, const char *path);

void fs2_sync(fs2_t *dst, fs2_t *src);

END_API
