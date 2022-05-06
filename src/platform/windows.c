#define I_WILL_BE_INCLUDING_PLATFORM_CODE

// clang-format off
#include "platform.h"
#include "cthulhu/util/macros.h"
#include "cthulhu/util/str.h"
// clang-format on

#define FORMAT_FLAGS (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)

STATIC_ASSERT(sizeof(TCHAR) == sizeof(char), "TCHAR and char must be the same size");
STATIC_ASSERT(sizeof(file_handle_t) == sizeof(void *), "file_handle_t and void* must be the same size");

// library api

library_handle_t native_library_open(const char *path, native_error_t *error)
{
    library_handle_t handle = LoadLibrary(path);

    if (handle == NULL)
    {
        *error = native_get_last_error();
    }

    return handle;
}

void native_library_close(library_handle_t handle)
{
    FreeLibrary(handle);
}

void *native_library_get_symbol(library_handle_t handle, const char *symbol, native_error_t *error)
{
    void *ptr = GetProcAddress(handle, symbol);

    if (ptr == NULL)
    {
        *error = native_get_last_error();
    }

    return ptr;
}

file_handle_t native_file_open(const char *path, file_mode_t mode, file_format_t format, native_error_t *error)
{
    DWORD access = (mode == MODE_READ) ? GENERIC_READ : GENERIC_WRITE;
    DWORD disposition = (mode == MODE_READ) ? OPEN_EXISTING : CREATE_ALWAYS;
    file_handle_t handle = CreateFile(
        /* lpFileName = */ path,
        /* dwDesiredAccess = */ access,
        /* dwShareMode = */ FILE_SHARE_READ,
        /* lpSecurityAttributes = */ NULL,
        /* dwCreationDisposition = */ disposition,
        /* dwFlagsAndAttributes = */ FILE_ATTRIBUTE_NORMAL,
        /* hTemplateFile = */ NULL);

    if (handle == INVALID_FILE_HANDLE)
    {
        *error = native_get_last_error();
    }

    return handle;
}

void native_file_close(file_handle_t handle)
{
    CloseHandle(handle);
}

file_read_t native_file_read(file_handle_t handle, void *buffer, file_read_t size, native_error_t *error)
{
    DWORD readSize;
    BOOL result = ReadFile(handle, buffer, size, &readSize, NULL);

    if (!result)
    {
        *error = native_get_last_error();
    }

    return readSize;
}

file_write_t native_file_write(file_handle_t handle, const void *buffer, file_write_t size, native_error_t *error)
{
    DWORD writtenSize = 0;
    BOOL result = WriteFile(handle, buffer, size, &writtenSize, NULL);

    if (!result)
    {
        *error = native_get_last_error();
    }

    return writtenSize;
}

file_size_t native_file_size(file_handle_t handle, native_error_t *error)
{
    LARGE_INTEGER size;
    BOOL result = GetFileSizeEx(handle, &size);

    if (!result)
    {
        *error = native_get_last_error();
    }

    return size.QuadPart;
}

const void *native_file_map(file_handle_t handle, native_error_t *error)
{
    HANDLE mapping = CreateFileMapping(
        /* hFile = */ handle,
        /* lpFileMappingAttributes = */ NULL,
        /* flProtect = */ PAGE_READONLY,
        /* dwMaximumSizeHigh = */ 0,
        /* dwMaximumSizeLow = */ 0,
        /* lpName = */ NULL);

    if (mapping == NULL)
    {
        *error = native_get_last_error();
        return NULL;
    }

    LPVOID file = MapViewOfFile(
        /* hFileMappingObject = */ mapping,
        /* dwDesiredAccess = */ FILE_MAP_READ,
        /* dwFileOffsetHigh = */ 0,
        /* dwFileOffsetLow = */ 0,
        /* dwNumberOfBytesToMap = */ 0);

    if (file == NULL)
    {
        *error = native_get_last_error();
        return NULL;
    }

    return file;
}

char *native_error_to_string(native_error_t error)
{
    char buffer[0x1000] = {0};

    DWORD written = FormatMessage(
        /* dwFlags = */ FORMAT_FLAGS,
        /* lpSource = */ NULL,
        /* dwMessageId = */ error,
        /* dwLanguageId = */ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        /* lpBuffer = */ buffer,
        /* nSize = */ sizeof(buffer),
        /* Arguments = */ NULL);

    if (written == 0)
    {
        return ctu_strdup("unknown error");
    }

    DWORD used = written;
    for (DWORD i = 0; i < used; i++)
    {
        if (buffer[i] == '\n' || buffer[i] == '\r')
        {
            memcpy(buffer + i, buffer + i + 1, written - i);
            used--;
            i--;
        }
    }

    return ctu_strndup(buffer, used);
}

native_error_t native_get_last_error(void)
{
    return GetLastError();
}
