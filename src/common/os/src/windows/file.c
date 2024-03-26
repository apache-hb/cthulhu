// SPDX-License-Identifier: LGPL-3.0-only

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

os_error_t impl_copyfile(const os_file_t *dst, const os_file_t *src)
{
    CTASSERT(dst != NULL);
    CTASSERT(src != NULL);

    BOOL result = CopyFileA(src->path, dst->path, FALSE);
    if (!result)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

static DWORD get_access(os_access_t access)
{
    DWORD result = 0;
    if (access & eOsAccessRead) { result |= GENERIC_READ; }
    if (access & eOsAccessWrite) { result |= GENERIC_WRITE; }
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

static DWORD get_disp(os_access_t access)
{
    switch (access)
    {
    case eOsAccessRead:
        return OPEN_EXISTING;

    case eOsAccessWrite:
    case (eOsAccessWrite | eOsAccessRead):
        return OPEN_ALWAYS;

    case (eOsAccessWrite | eOsAccessTruncate):
    case (eOsAccessWrite | eOsAccessRead | eOsAccessTruncate):
        return CREATE_ALWAYS;

    default:
        CT_NEVER("invalid access flags %s", os_access_string(access));
    }
}

USE_DECL
os_error_t os_file_open(const char *path, os_access_t access, os_file_t *file)
{
    CTASSERT(path != NULL);
    CTASSERT(file != NULL);
    CTASSERTF(access & (eOsAccessRead | eOsAccessWrite), "%s: invalid access flags 0x%x", path, access);
    CTASSERTF(access != (eOsAccessRead | eOsAccessTruncate), "%s: cannot truncate read only file", path);

    DWORD dw_access = get_access(access);
    DWORD dw_disp = get_disp(access);

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
        .file = handle
    };

    *file = result;
    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_tmpfile_open(os_file_t *file)
{
    CTASSERT(file != NULL);

    char path[MAX_PATH];
    DWORD result = GetTempPathA(MAX_PATH, path);
    if (result == 0)
    {
        return GetLastError();
    }

    char name[MAX_PATH];
    result = GetTempFileNameA(path, "ctu", 0, name);
    if (result == 0)
    {
        return GetLastError();
    }

    return os_file_open(name, eOsAccessWrite, file);
}

USE_DECL
os_error_t os_file_close(os_file_t *fd)
{
    CTASSERT(fd != NULL);

    if (CloseHandle(fd->file) == 0)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_file_read(os_file_t *file, void *buffer, size_t size, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERT(actual != NULL);
    CTASSERTF(size <= UINT32_MAX, "cannot read more than %u bytes at once (%zu is too big)", UINT32_MAX, size);

    DWORD read_size = 0;
    BOOL result = ReadFile(file->file, buffer, (DWORD)size, &read_size, NULL);

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
    CTASSERT(actual != NULL);
    CTASSERTF(size <= UINT32_MAX, "cannot write more than %u bytes at once (%zu is too big)", UINT32_MAX, size);

    DWORD written_size = 0;
    BOOL result = WriteFile(file->file, buffer, (DWORD)size, &written_size, NULL);

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
    BOOL result = GetFileSizeEx(file->file, &size);
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
    BOOL result = SetFilePointerEx(file->file, it, &out, FILE_BEGIN);
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
    BOOL result = SetFilePointerEx(file->file, zero, &offset, FILE_CURRENT);
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
    BOOL result = SetFilePointerEx(file->file, it, NULL, FILE_BEGIN);
    if (!result)
    {
        return GetLastError();
    }

    result = SetEndOfFile(file->file);
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
    case eOsProtectNone: return PAGE_NOACCESS; // none maps to noaccess
    case eOsProtectRead: return PAGE_READONLY; // readonly
    case eOsProtectExecute: return PAGE_EXECUTE; // execute only

    // we map both write and readwrite to PAGE_READWRITE
    // i want to avoid copyonwrite
    case eOsProtectWrite:
    case (eOsProtectRead | eOsProtectWrite):
        return PAGE_READWRITE;

    case (eOsProtectRead | eOsProtectExecute):
        return PAGE_EXECUTE_READ;

    // wx and rwx are mapped to PAGE_EXECUTE_READWRITE
    // again, avoiding copyonwrite
    case (eOsProtectWrite | eOsProtectExecute):
    case (eOsProtectRead | eOsProtectWrite | eOsProtectExecute):
        return PAGE_EXECUTE_READWRITE;

    default:
        CT_NEVER("unknown protect %s", os_protect_string(protect));
    }
}

static DWORD get_map_access(os_protect_t protect)
{
    // TODO: FILE_MAP_LARGE_PAGES would be fun to play with here for large files

    DWORD result = 0;
    if (protect & eOsProtectRead) { result |= FILE_MAP_READ; }
    if (protect & eOsProtectWrite) { result |= FILE_MAP_WRITE; }
    if (protect & eOsProtectExecute) { result |= FILE_MAP_EXECUTE; }
    return result;
}

USE_DECL
os_error_t os_file_map(os_file_t *file, os_protect_t protect, size_t size, os_mapping_t *mapping)
{
    CTASSERT(file != NULL);
    CTASSERT(mapping != NULL);
    CTASSERT(size > 0);

    DWORD prot = get_protect(protect);
    DWORD access = get_map_access(protect);

    DWORD high = (DWORD)(size >> 32);
    DWORD low = (DWORD)(size & 0xFFFFFFFF);

    HANDLE handle = CreateFileMapping(
        /* hFile = */ file->file,
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
os_error_t os_file_unmap(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    if (!UnmapViewOfFile(mapping->view))
    {
        return GetLastError();
    }

    if (!CloseHandle(mapping->handle))
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}
