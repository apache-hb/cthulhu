// SPDX-License-Identifier: LGPL-3.0-only

#define _POSIX_C_SOURCE 200112L

#include "os/core.h"
#include "os_common.h"

#include "base/util.h"
#include "std/str.h"
#include "core/macros.h"

#include <string.h>
#include <errno.h>

STA_DECL
CT_LOCAL os_error_t impl_last_error(void)
{
    return (os_error_t)errno;
}

CT_LOCAL size_t impl_error_length(os_error_t error)
{
    CT_UNUSED(error);
    return 256; // TODO: find a way to get the size of the buffer
}

CT_LOCAL size_t impl_error_string(os_error_t error, char *buffer, size_t size)
{
    int err = (int)error;
    if (strerror_r(err, buffer, size) != 0)
    {
        return str_sprintf(buffer, size, "errno: %d", err);
    }

    return ctu_strlen(buffer);
}
