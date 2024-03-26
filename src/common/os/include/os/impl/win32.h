// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/win32.h" // IWYU pragma: export

#include <windef.h>
#include <minwinbase.h>

typedef HMODULE os_library_impl_t;
typedef HANDLE os_file_impl_t;

typedef struct os_mapping_t
{
    HANDLE handle;
    LPVOID view;
    size_t size;
} os_mapping_t;

typedef struct os_iter_t
{
    HANDLE find;
    WIN32_FIND_DATA data;
    DWORD error;
} os_iter_t;

typedef struct os_inode_t
{
    WIN32_FIND_DATA data;
} os_inode_t;

enum {
    eOsSuccess = ERROR_SUCCESS,
    eOsNotFound = ERROR_FILE_NOT_FOUND,
    eOsExists = ERROR_ALREADY_EXISTS,
    eOsTooSmall = ERROR_DISK_FULL,
};

#define CT_OS_INVALID_FILE INVALID_HANDLE_VALUE
#define CT_OS_INVALID_LIBRARY NULL
#define CT_OS_INVALID_MAPPING NULL

#define CT_OS_NAME_MAX MAX_PATH
