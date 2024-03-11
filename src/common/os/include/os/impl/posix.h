// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <stdio.h>
#include <dirent.h>
#include <errno.h>

typedef struct os_file_t
{
    const char *path;
    FILE *file;
} os_file_t;

typedef struct os_mapping_t
{
    void *data;
    size_t size;
} os_mapping_t;

typedef struct os_iter_t
{
    DIR *dir;
    int error;
} os_iter_t;

typedef struct os_dir_t
{
    struct dirent *ent;
} os_dir_t;

typedef struct os_library_t
{
    void *library;
} os_library_t;

enum {
    eOsSuccess = 0,
    eOsNotFound = ENOENT,
};
