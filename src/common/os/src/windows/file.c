#include "base/panic.h"

#include "os/os.h"
#include <stdint.h>

static DWORD get_access(os_access_t access)
{
    DWORD result = 0;
    if (access & eAccessRead) { result |= GENERIC_READ; }
    if (access & eAccessWrite) { result |= GENERIC_WRITE; }
    return result;
}

USE_DECL
os_error_t os_file_exists(const char *path, bool *exists)
{
    CTASSERT(path != NULL);
    CTASSERT(exists != NULL);

    DWORD attr = GetFileAttributes(path);
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
        {
            *exists = false;
            return ERROR_SUCCESS;
        }
        else
        {
            return error;
        }
    }

    *exists = true;
    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_file_open(const char *path, os_access_t access, os_file_t *file)
{
    CTASSERT(path != NULL);
    CTASSERT(access & (eAccessRead | eAccessWrite));
    CTASSERT(file != NULL);

    DWORD dw_access = get_access(access);
    DWORD dw_disp = (access & eAccessWrite)
        ? CREATE_ALWAYS
        : OPEN_EXISTING;

    HANDLE handle = CreateFile(
        /* lpFileName = */ path,
        /* dwDesiredAccess = */ dw_access,
        /* dwShareMode = */ FILE_SHARE_READ,
        /* lpSecurityAttributes = */ NULL,
        /* dwCreationDisposition = */ dw_disp,
        /* dwFlagsAndAttributes = */ FILE_ATTRIBUTE_NORMAL,
        /* hTemplateFile = */ NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    os_file_t result = {
        .path = path,
        .handle = handle
    };

    *file = result;
    return ERROR_SUCCESS;
}

USE_DECL
void os_file_close(os_file_t *fd)
{
    CTASSERT(fd != NULL);

    CloseHandle(fd->handle); // TODO: check result
}

USE_DECL
os_error_t os_file_read(os_file_t *file, void *buffer, size_t size, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERTF(size > 0 && size <= UINT32_MAX, "size=%zu", size);
    CTASSERT(actual != NULL);

    DWORD read_size = 0;
    BOOL result = ReadFile(file->handle, buffer, (DWORD)size, &read_size, NULL);

    size_t read = read_size;

    if (!result)
    {
        return GetLastError();
    }

    *actual = read;
    return 0;
}

USE_DECL
os_error_t os_file_write(os_file_t *file, const void *buffer, size_t size, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERT(size > 0 && size <= UINT32_MAX);
    CTASSERT(actual != NULL);

    DWORD written_size = 0;
    BOOL result = WriteFile(file->handle, buffer, (DWORD)size, &written_size, NULL);

    size_t written = written_size;

    if (!result)
    {
        return GetLastError();
    }

    *actual = written;
    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_file_size(os_file_t *file, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(actual != NULL);

    LARGE_INTEGER size;
    BOOL result = GetFileSizeEx(file->handle, &size);
    *actual = size.QuadPart;

    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_file_seek(os_file_t *file, size_t offset, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(actual != NULL);

    LARGE_INTEGER it = { .QuadPart = (LONGLONG)offset };
    LARGE_INTEGER out = { 0 };
    BOOL result = SetFilePointerEx(file->handle, it, &out, FILE_BEGIN);
    *actual = out.QuadPart;

    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_file_tell(os_file_t *file, size_t *actual)
{
    CTASSERT(file != NULL);

    LARGE_INTEGER offset = { 0 };
    LARGE_INTEGER zero = { 0 };
    BOOL result = SetFilePointerEx(file->handle, zero, &offset, FILE_CURRENT);
    *actual = offset.QuadPart;

    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_file_map(os_file_t *file, const void **mapped)
{
    CTASSERT(file != NULL);
    CTASSERT(mapped != NULL);

    // TODO: maybe mapping should be part of os_file_t?
    HANDLE mapping = CreateFileMapping(
        /* hFile = */ file->handle,
        /* lpFileMappingAttributes = */ NULL,
        /* flProtect = */ PAGE_READONLY,
        /* dwMaximumSizeHigh = */ 0,
        /* dwMaximumSizeLow = */ 0,
        /* lpName = */ NULL);

    if (mapping == NULL)
    {
        return GetLastError();
    }

    LPVOID view = MapViewOfFile(
        /* hFileMappingObject = */ mapping,
        /* dwDesiredAccess = */ FILE_MAP_READ,
        /* dwFileOffsetHigh = */ 0,
        /* dwFileOffsetLow = */ 0,
        /* dwNumberOfBytesToMap = */ 0);

    if (view == NULL)
    {
        return GetLastError();
    }

    *mapped = view;
    return ERROR_SUCCESS;
}

USE_DECL
const char *os_file_name(os_file_t *file)
{
    CTASSERT(file != NULL);

    return file->path;
}
