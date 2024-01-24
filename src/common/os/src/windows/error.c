#include "os/os.h"

#include "std/str.h"

#include "core/win32.h" // IWYU pragma: keep

#if CTU_WIN32_TRICKERY
#   include <winbase.h>
#endif

#define FORMAT_FLAGS (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)

USE_DECL
char *os_error_string(os_error_t error, arena_t *arena)
{
    char buffer[0x1000];

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
        return str_format(arena, "unknown error (0x%08lX)", (DWORD)error);
    }

    char *cleaned = str_erase(buffer, written, "\n\r.");
    return str_format(arena, "%s (0x%08lX)", cleaned, (DWORD)error);
}
