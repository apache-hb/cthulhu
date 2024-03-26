// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <stdio.h>
#include <dirent.h>
#include <errno.h>

typedef void *os_library_impl_t;
typedef FILE *os_file_impl_t;

typedef struct os_mapping_t
{
    void *view;
    size_t size;
} os_mapping_t;

typedef struct os_iter_t
{
    DIR *dir;
    int error;
} os_iter_t;

typedef struct os_inode_t
{
    struct dirent *ent;
} os_inode_t;

enum {
    eOsSuccess = 0,
    eOsNotFound = ENOENT,
    eOsExists = EEXIST,
};

#define CT_OS_INVALID_FILE NULL
#define CT_OS_INVALID_MAPPING NULL

#define CT_OS_NAME_MAX NAME_MAX
