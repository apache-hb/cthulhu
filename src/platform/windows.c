#include "src/platform/platform.h"
#include "cthulhu/util/macros.h"
#include "cthulhu/util/str.h"

#define FORMAT_FLAGS (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)

STATIC_ASSERT(sizeof(TCHAR) == sizeof(char), "TCHAR and char must be the same size");

// library api

library_handle_t native_library_open(const char *path, error_t *error)
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

void *native_library_get_symbol(library_handle_t handle, const char *symbol, error_t *error)
{
    void *ptr = GetProcAddress(handle, symbol);

    if (ptr == NULL)
    {
        *error = native_get_last_error();
    }

    return ptr;
}

char *native_error_to_string(error_t error)
{
    char buffer[0x1000] = { 0 };

    DWORD written = FormatMessage(
        FORMAT_FLAGS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        sizeof(buffer),
        NULL);

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
        }
    }

    return ctu_strndup(buffer, used);
}

error_t native_get_last_error(void)
{
    return GetLastError();
}
