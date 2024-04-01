// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"
#include "os_common.h"

#include "std/str.h"
#include "base/panic.h"
#include "base/util.h"

#include "core/win32.h" // IWYU pragma: keep

#define FORMAT_FLAGS (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)

os_error_t impl_last_error(void)
{
    return (os_error_t)GetLastError();
}

static DWORD format_inner(os_error_t error, char *buffer, size_t size)
{
    return FormatMessage(
        /* dwFlags = */ FORMAT_FLAGS,
        /* lpSource = */ NULL,
        /* dwMessageId = */ (DWORD)error,
        /* dwLanguageId = */ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        /* lpBuffer = */ buffer,
        /* nSize = */ (DWORD)size,
        /* Arguments = */ NULL);
}

USE_DECL
size_t os_error_get_string(os_error_t error, char *buffer, size_t size)
{
    if (size == 0)
    {
        // caller is asking for the size of the buffer
        CTASSERT(buffer == NULL);

        // TODO: is there a way of asking for the size of the buffer at all?
        // this is what .NET core does, but its not clear if this is correct.
        return 512 * sizeof(TCHAR);
    }

    CTASSERT(buffer != NULL);
    DWORD written = format_inner(error, buffer, size);

    if (written == 0)
    {
        return str_sprintf(buffer, size, "unknown error (0x%08lX)", (DWORD)error);
    }

    // replace every instance of \n\r\t with a single space
    // if there are consecutive newlines or whitespace, replace them with a single space.
    text_t text = text_make(buffer, written);
    str_replace_inplace(&text, "\r\n\t", " ");

    // trim trailing whitespace and . characters
    str_trim_back_inplace(&text, "\r\n\t .");

    return text.length;
}
