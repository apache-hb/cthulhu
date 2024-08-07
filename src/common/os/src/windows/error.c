// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"
#include "os_common.h"

#include "std/str.h"
#include "base/util.h"

#include "core/macros.h"
#include "core/win32.h" // IWYU pragma: keep

#define FORMAT_FLAGS (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)

STA_DECL
CT_LOCAL os_error_t impl_last_error(void)
{
    return (os_error_t)GetLastError();
}

static DWORD format_inner(os_error_t error, char *buffer, size_t size)
{
    return FormatMessageA(
        /* dwFlags = */ FORMAT_FLAGS,
        /* lpSource = */ NULL,
        /* dwMessageId = */ (DWORD)error,
        /* dwLanguageId = */ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        /* lpBuffer = */ buffer,
        /* nSize = */ (DWORD)size,
        /* Arguments = */ NULL);
}

CT_LOCAL size_t impl_error_length(os_error_t error)
{
    // TODO: is there a way of asking for the size of the buffer at all?
    // this is what .NET core does, but its not clear if this is correct.
    CT_UNUSED(error);
    return 1024;
}

CT_LOCAL size_t impl_error_string(os_error_t error, char *buffer, size_t size)
{
    DWORD written = format_inner(error, buffer, size);

    if (written == 0)
    {
        return str_sprintf(buffer, size, "GetLastError: 0x%08lX", (DWORD)error);
    }

    // replace every instance of \n\r\t with a single space
    // if there are consecutive newlines or whitespace, replace them with a single space.
    text_t text = text_make(buffer, written);
    str_replace_inplace(&text, "\r\n\t", " ");

    // trim trailing whitespace and . characters
    str_trim_back_inplace(&text, "\r\n\t .");

    return text.length;
}
