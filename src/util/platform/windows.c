#include "cthulhu/util/error.h"
#include "cthulhu/util/str.h"

LPTSTR ctu_err_string(DWORD err)
{
    DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;

    LPTSTR str = NULL;

    FormatMessage(
        /* dwFlags = */ flags,
        /* lpSource = */ NULL,
        /* dwMessageId = */ err,
        /* dwLanguageId = */ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        /* lpBuffer = */ (LPTSTR)&str,
        /* nSize = */ 0,
        /* Arguments = */ NULL);

    if (str == NULL)
    {
        str = "unknown error";
    }

    str = str_replace(str, "\n", "");
    str = str_replace(str, "\r", "");

    return format("%s (0x%x)", str, err);
}
