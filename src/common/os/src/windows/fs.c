// SPDX-License-Identifier: LGPL-3.0-only

#include "base/panic.h"

#include "core/win32.h" // IWYU pragma: keep

#include "os/os.h"

USE_DECL
os_error_t os_file_delete(const char *path)
{
    BOOL result = DeleteFileA(path);

    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_dir_create(const char *path)
{
    CTASSERT(path != NULL);

    BOOL result = CreateDirectoryA(path, NULL);
    if (!result)
    {
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS)
        {
            return eOsExists;
        }

        return error;
    }

    return eOsSuccess;
}

USE_DECL
os_error_t os_dir_delete(const char *path)
{
    BOOL result = RemoveDirectoryA(path);
    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_dirent_t os_dirent_type(const char *path)
{
    DWORD attributes = GetFileAttributesA(path);

    if (attributes != INVALID_FILE_ATTRIBUTES)
    {
        // TODO: this doesnt handle pipes, sockets, etc.
        return (attributes & FILE_ATTRIBUTE_DIRECTORY)
            ? eOsNodeDir
            : eOsNodeFile;
    }

    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
    {
        return eOsNodeNone;
    }

    return eOsNodeError;
}

USE_DECL
size_t os_cwd_get_string(char *buffer, size_t size)
{
    if (size == 0)
        CTASSERT(buffer == NULL);
    else
        CTASSERT(buffer != NULL);

    return GetCurrentDirectoryA((DWORD)size, buffer);
}
