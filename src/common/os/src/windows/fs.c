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

// TODO: this feels janky
USE_DECL
os_error_t os_file_create(const char *path)
{
    HANDLE handle = CreateFile(
        /* lpFileName = */ path,
        /* dwDesiredAccess = */ GENERIC_WRITE,
        /* dwShareMode = */ FILE_SHARE_READ,
        /* lpSecurityAttributes = */ NULL,
        /* dwCreationDisposition = */ CREATE_NEW,
        /* dwFlagsAndAttributes = */ FILE_ATTRIBUTE_NORMAL,
        /* hTemplateFile = */ NULL);

    bool valid = handle != INVALID_HANDLE_VALUE;

    if (!valid)
    {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS)
        {
            return error;
        }
    }
    else
    {
        CloseHandle(handle);
    }

    return ERROR_SUCCESS;
}

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
bool os_dir_exists(const char *path)
{
    os_dirent_t type = os_dirent_type(path);
    return type == eOsNodeDir;
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
os_error_t os_dir_current(char *cwd, size_t size)
{
    CTASSERT(cwd != NULL);
    CTASSERT(size > 0);

    DWORD ret = GetCurrentDirectoryA((DWORD)size, cwd);

    if (ret == 0)
    {
        return GetLastError();
    }

    // add null terminator
    cwd[ret] = '\0';
    return ERROR_SUCCESS;
}
