#include "common.h"

#include "os/os.h"

#include "memory/memory.h"

// TODO: this feels janky
USE_DECL
OS_RESULT(bool) os_file_create(const char *path)
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
            return win_error(error);
        }
    }
    else
    {
        CloseHandle(handle);
    }

    return win_result(ERROR_SUCCESS, &valid, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_file_delete(const char *path)
{
    bool result = DeleteFileA(path);

    if (!result)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &result, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_file_exists(const char *path)
{
    OS_RESULT(os_dirent_t) type = os_dirent_type(path);
    if (os_error(type))
    {
        return type;
    }

    bool result = OS_VALUE(os_dirent_t, type) == eOsNodeFile;
    return win_result(ERROR_SUCCESS, &result, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_dir_create(const char *path)
{
    bool result = CreateDirectoryA(path, NULL);
    if (!result)
    {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS)
        {
            return win_error(error);
        }

        result = true;
    }

    return win_result(ERROR_SUCCESS, &result, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_dir_delete(const char *path)
{
    bool result = RemoveDirectoryA(path);
    if (!result)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &result, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_dir_exists(const char *path)
{
    OS_RESULT(os_dirent_t) type = os_dirent_type(path);
    if (os_error(type)) { return type; }

    os_dirent_t ent = OS_VALUE(os_dirent_t, type);
    bool result = ent == eOsNodeDir;
    return win_result(ERROR_SUCCESS, &result, sizeof(bool));
}

USE_DECL
OS_RESULT(os_dirent_t) os_dirent_type(const char *path)
{
    DWORD attributes = GetFileAttributesA(path);

    if (attributes != INVALID_FILE_ATTRIBUTES)
    {
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            os_dirent_t ent = eOsNodeDir;
            return win_result(ERROR_SUCCESS, &ent, sizeof(os_dirent_t));
        }
        else
        {
            os_dirent_t ent = eOsNodeFile;
            return win_result(ERROR_SUCCESS, &ent, sizeof(os_dirent_t));
        }
    }

    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
    {
        os_dirent_t ent = eOsNodeNone;
        return win_result(ERROR_SUCCESS, &ent, sizeof(os_dirent_t));
    }

    return win_error(error);
}

USE_DECL
OS_RESULT(const char *) os_dir_current(void)
{
    DWORD size = GetCurrentDirectoryA(0, NULL);
    char *buffer = ctu_malloc(size + 1);

    if (GetCurrentDirectoryA(size, buffer) == 0)
    {
        return win_error(GetLastError());
    }

    // add null terminator
    buffer[size] = '\0';

    return win_result(ERROR_SUCCESS, &buffer, sizeof(char *));
}
