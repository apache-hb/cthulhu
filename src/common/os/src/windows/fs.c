// SPDX-License-Identifier: LGPL-3.0-only

#include "base/panic.h"

#include "core/win32.h" // IWYU pragma: keep

#include "os/os.h"

#if CTU_WIN32_TRICKERY
#   include <errhandlingapi.h>
#   include <fileapi.h>
#   include <handleapi.h>
#   include <processenv.h>
#   include <winerror.h>
#endif

USE_DECL
os_error_t os_file_delete(const char *path)
{
    bool result = DeleteFileA(path);

    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_dir_create(const char *path, bool *create)
{
    CTASSERT(path != NULL);
    CTASSERT(create != NULL);

    bool result = CreateDirectoryA(path, NULL);
    if (!result)
    {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS)
        {
            return error;
        }

        result = true;
    }

    *create = result;
    return 0;
}

USE_DECL
os_error_t os_dir_delete(const char *path)
{
    bool result = RemoveDirectoryA(path);
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
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return eOsNodeDir;
        }
        else
        {
            return eOsNodeFile;
        }
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
