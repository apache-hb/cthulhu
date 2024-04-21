// SPDX-License-Identifier: LGPL-3.0-only

#define _POSIX_C_SOURCE 200112L

#include "os/core.h"
#include "os_common.h"

#include "base/util.h"
#include "std/str.h"

#include <string.h>
#include <errno.h>

os_error_t impl_last_error(void)
{
    return (os_error_t)errno;
}

USE_DECL
size_t os_error_get_string(os_error_t error, char *buffer, size_t size)
{
    if (size == 0)
    {
        // caller is asking for the size of the buffer
        return 256; // TODO: find a way to get the size of the buffer
    }

    int err = (int)error;
    if (strerror_r(err, buffer, size) != 0)
    {
        return str_sprintf(buffer, size, "errno: %d", err);
    }

    return ctu_strlen(buffer);
}
