#include "os/error.h"

#include "std/str.h"

#include "core/win32.h"

#define FORMAT_FLAGS (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)

USE_DECL
const char *os_error_string(os_error_t error)
{
    char buffer[0x1000] = {0};

    DWORD written = FormatMessage(
        /* dwFlags = */ FORMAT_FLAGS,
        /* lpSource = */ NULL,
        /* dwMessageId = */ (DWORD)error,
        /* dwLanguageId = */ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        /* lpBuffer = */ buffer,
        /* nSize = */ sizeof(buffer),
        /* Arguments = */ NULL);

    if (written == 0)
    {
        return format("unknown error (0x%08lX)", (DWORD)error);
    }

    char *cleaned = str_erase(buffer, written, "\n\r.");
    return format("%s (0x%08lX)", cleaned, (DWORD)error);
}
