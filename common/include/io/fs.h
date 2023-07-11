#pragma once

#include "io/io.h"

BEGIN_API

typedef struct reports_t reports_t;

typedef struct fs_t fs_t;

fs_t *fs_physical(reports_t *reports, const char *root);
fs_t *fs_virtual(reports_t *reports, const char *name);

void fs_dir_create(fs_t *fs, const char *path);
void fs_file_create(fs_t *fs, const char *path);

io_t *fs_open(fs_t *fs, const char *path, os_access_t flags);

bool fs_dir_exists(fs_t *fs, const char *path);
bool fs_file_exists(fs_t *fs, const char *path);

void fs_dir_delete(fs_t *fs, const char *path);
void fs_file_delete(fs_t *fs, const char *path);

void fs_dir_create_recursive(fs_t *fs, const char *path);

void fs_sync(fs_t *dst, fs_t *src);

END_API
