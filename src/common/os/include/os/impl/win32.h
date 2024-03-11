// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/win32.h" // IWYU pragma: export

#include <windef.h>
#include <minwinbase.h>

typedef struct os_file_t
{
    const char *path;
    HANDLE handle;
} os_file_t;

typedef struct os_mapping_t
{
    HANDLE handle;
    LPVOID view;
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

typedef struct os_library_t
{
    HMODULE library;
} os_library_t;

enum {
    eOsSuccess = ERROR_SUCCESS,
    eOsNotFound = ERROR_FILE_NOT_FOUND,
    eOsExists = ERROR_ALREADY_EXISTS,
};
