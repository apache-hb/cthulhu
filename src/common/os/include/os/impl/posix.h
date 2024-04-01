// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <errno.h>
#include <stdio.h>
#include <dirent.h>

typedef void *os_library_impl_t;
typedef FILE *os_file_impl_t;
typedef struct dirent *os_inode_impl_t;
typedef DIR *os_iter_impl_t;

typedef struct os_mapping_t
{
    void *view;
    size_t size;
} os_mapping_t;

enum {
    eOsSuccess = 0,
    eOsNotFound = ENOENT,
    eOsExists = EEXIST,
    eOsTooSmall = ENOSPC,
};

#define CT_OS_INVALID_FILE NULL
#define CT_OS_INVALID_LIBRARY NULL

// TODO: this may actually be UINTPTR_MAX
#define CT_OS_INVALID_MAPPING NULL
#define CT_OS_INVALID_ITER NULL

// TODO: this is wrong, im not sure how i might go about fixing it
#ifdef _DIRENT_HAVE_D_NAMLEN
#   define CT_OS_NAME_MAX NAME_MAX
#else
#   define CT_OS_NAME_MAX 255
#endif
