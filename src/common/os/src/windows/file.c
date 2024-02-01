#include "base/panic.h"

#include "os/os.h"

#include <stdint.h>

#if CTU_WIN32_TRICKERY
#   include <errhandlingapi.h>
#   include <handleapi.h>
#   include <fileapi.h>
#   include <winbase.h>
#   include <winerror.h>
#endif

USE_DECL
os_error_t os_file_copy(const char *src, const char *dst)
{
    CTASSERT(src != NULL);
    CTASSERT(dst != NULL);

    // use CopyFile, i dont think we have a reason to need CopyFileEx

    BOOL result = CopyFileA(src, dst, FALSE);
    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

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
    CTASSERTF(size > 0 && size <= UINT32_MAX, "size %zu out of range", size);
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
os_error_t os_file_expand(os_file_t *file, size_t size)
{
    CTASSERT(file != NULL);

    LARGE_INTEGER it = { .QuadPart = (LONGLONG)size };
    BOOL result = SetFilePointerEx(file->handle, it, NULL, FILE_BEGIN);
    if (!result)
    {
        return GetLastError();
    }

    result = SetEndOfFile(file->handle);
    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

static DWORD get_protect(os_protect_t protect)
{
    // windows PAGE_ macros dont follow a bit pattern
    // so we end up with a pretty big pile of conditionals
    switch (protect)
    {
    case eProtectNone: return PAGE_NOACCESS; // none maps to noaccess
    case eProtectRead: return PAGE_READONLY; // readonly
    case eProtectExecute: return PAGE_EXECUTE; // execute only

    // we map both write and readwrite to PAGE_READWRITE
    // i want to avoid copyonwrite
    case eProtectWrite:
    case (eProtectRead | eProtectWrite):
        return PAGE_READWRITE;

    case (eProtectRead | eProtectExecute):
        return PAGE_EXECUTE_READ;

    // wx and rwx are mapped to PAGE_EXECUTE_READWRITE
    // again, avoiding copyonwrite
    case (eProtectWrite | eProtectExecute):
    case (eProtectRead | eProtectWrite | eProtectExecute):
        return PAGE_EXECUTE_READWRITE;

        // special cases

    default: NEVER("unknown protect %x", protect);
    }
}

static DWORD get_map_access(os_protect_t protect)
{
    // TODO: FILE_MAP_LARGE_PAGES would be fun to play with here for large files

    DWORD result = 0;
    if (protect & eProtectRead) { result |= FILE_MAP_READ; }
    if (protect & eProtectWrite) { result |= FILE_MAP_WRITE; }
    if (protect & eProtectExecute) { result |= FILE_MAP_EXECUTE; }
    return result;
}

USE_DECL
os_error_t os_file_map(os_file_t *file, os_protect_t protect, size_t size, os_mapping_t *mapping)
{
    CTASSERT(file != NULL);
    CTASSERT(mapping != NULL);
    // TODO: how should we constrain size?

    DWORD prot = get_protect(protect);
    DWORD access = get_map_access(protect);

    DWORD high = (DWORD)(size >> 32);
    DWORD low = (DWORD)(size & 0xFFFFFFFF);

    HANDLE handle = CreateFileMapping(
        /* hFile = */ file->handle,
        /* lpFileMappingAttributes = */ NULL,
        /* flProtect = */ prot,
        /* dwMaximumSizeHigh = */ high,
        /* dwMaximumSizeLow = */ low,
        /* lpName = */ NULL);

    if (handle == NULL)
    {
        return GetLastError();
    }

    LPVOID view = MapViewOfFile(
        /* hFileMappingObject = */ handle,
        /* dwDesiredAccess = */ access,
        /* dwFileOffsetHigh = */ 0,
        /* dwFileOffsetLow = */ 0,
        /* dwNumberOfBytesToMap = */ 0); // view the whole mapping

    if (view == NULL)
    {
        return GetLastError();
    }

    os_mapping_t result = {
        .handle = handle,
        .view = view
    };

    *mapping = result;

    return ERROR_SUCCESS;
}

USE_DECL
void os_file_unmap(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    // TODO: not sure if we should check the result of these
    UnmapViewOfFile(mapping->view);
    CloseHandle(mapping->handle);
}

USE_DECL
void *os_mapping_data(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    return mapping->view;
}

USE_DECL
bool os_mapping_active(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    return mapping->view != NULL;
}

USE_DECL
const char *os_file_name(os_file_t *file)
{
    CTASSERT(file != NULL);

    return file->path;
}
