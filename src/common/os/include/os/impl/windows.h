#pragma once

#include "core/win32.h" // IWYU pragma: export

#include <windef.h>
#include <minwinbase.h>

typedef struct os_file_t
{
    const char *path;
    HANDLE handle;
} os_file_t;

typedef struct os_iter_t
{
    HANDLE find;
    WIN32_FIND_DATA data;
    DWORD error;
} os_iter_t;

typedef struct os_dir_t
{
    WIN32_FIND_DATA data;
} os_dir_t;

typedef struct os_library_t
{
    HMODULE library;
} os_library_t;
