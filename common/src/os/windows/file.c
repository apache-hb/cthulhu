#include "common.h"

#include "base/panic.h"
#include "base/util.h"

#include <limits.h>

typedef struct os_file_t
{
    const char *path;
    HANDLE handle;
} os_file_t;

static DWORD get_access(os_access_t access)
{
    DWORD result = 0;
    if (access & eAccessRead) { result |= GENERIC_READ; }
    if (access & eAccessWrite) { result |= GENERIC_WRITE; }
    return result;
}

OS_RESULT(os_file_t *) os_file_open(const char *path, os_access_t access)
{
    CTASSERT(path != NULL);
    CTASSERT(access & (eAccessRead | eAccessWrite));
    DWORD dwAccess = get_access(access);
    DWORD dwDisposition = (access & eAccessWrite)
        ? (OPEN_ALWAYS | TRUNCATE_EXISTING)
        : OPEN_EXISTING;
    HANDLE handle = CreateFile(
        /* lpFileName = */ path,
        /* dwDesiredAccess = */ dwAccess,
        /* dwShareMode = */ FILE_SHARE_READ,
        /* lpSecurityAttributes = */ NULL,
        /* dwCreationDisposition = */ dwDisposition,
        /* dwFlagsAndAttributes = */ FILE_ATTRIBUTE_NORMAL,
        /* hTemplateFile = */ NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return win_error(GetLastError());
    }

    os_file_t fd = {
        .path = path,
        .handle = handle,
    };

    return win_result(ERROR_SUCCESS, &fd, sizeof(os_file_t));
}

void os_file_close(os_file_t *fd)
{
    CTASSERT(fd != NULL);
    CloseHandle(fd->handle); // TODO: check result
}

OS_RESULT(size_t) os_file_read(os_file_t *file, void *buffer, size_t size)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERTF(size > 0 && size <= UINT32_MAX, "size=%zu", size);

    DWORD readSize = 0;
    BOOL result = ReadFile(file->handle, buffer, (DWORD)size, &readSize, NULL);

    size_t read = readSize;

    if (!result)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &read, sizeof(size_t));
}

OS_RESULT(size_t) os_file_write(os_file_t *file, const void *buffer, size_t size)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERT(size > 0 && size <= UINT32_MAX);

    DWORD writtenSize = 0;
    BOOL result = WriteFile(file->handle, buffer, (DWORD)size, &writtenSize, NULL);

    size_t written = writtenSize;

    if (!result)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &written, sizeof(size_t));
}

OS_RESULT(size_t) os_file_size(os_file_t *file)
{
    CTASSERT(file != NULL);

    LARGE_INTEGER size;
    BOOL result = GetFileSizeEx(file->handle, &size);
    size_t it = size.QuadPart;

    if (!result)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &it, sizeof(size_t));
}

OS_RESULT(size_t) os_file_seek(os_file_t *file, size_t offset)
{
    CTASSERT(file != NULL);

    LARGE_INTEGER it = { .QuadPart = offset };
    LARGE_INTEGER out = { 0 };
    BOOL result = SetFilePointerEx(file->handle, it, &out, FILE_BEGIN);
    size_t seek = out.QuadPart;

    if (!result)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &seek, sizeof(size_t));
}

OS_RESULT(size_t) os_file_tell(os_file_t *file)
{
    CTASSERT(file != NULL);

    LARGE_INTEGER offset = { 0 };
    LARGE_INTEGER zero = { 0 };
    BOOL result = SetFilePointerEx(file->handle, zero, &offset, FILE_CURRENT);
    size_t it = offset.QuadPart;

    if (!result)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &it, sizeof(size_t));
}

OS_RESULT(const void *) os_file_map(os_file_t *file)
{
    CTASSERT(file != NULL);

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
        return win_error(GetLastError());
    }

    LPVOID view = MapViewOfFile(
        /* hFileMappingObject = */ mapping,
        /* dwDesiredAccess = */ FILE_MAP_READ,
        /* dwFileOffsetHigh = */ 0,
        /* dwFileOffsetLow = */ 0,
        /* dwNumberOfBytesToMap = */ 0);

    if (view == NULL)
    {
        return win_error(GetLastError());
    }

    return win_result(ERROR_SUCCESS, &view, sizeof(LPVOID));
}

const char *os_file_name(os_file_t *file)
{
    CTASSERT(file != NULL);
    return file->path;
}
