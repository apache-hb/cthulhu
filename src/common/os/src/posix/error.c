// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"

#include "std/str.h"

#include <string.h>

USE_DECL
char *os_error_string(os_error_t error, arena_t *arena)
{
    int err = (int)error;

    const char *str = strerror(err);
    if (str == NULL)
    {
        return str_format(arena, "errno: %d", err);
    }

    return str_format(arena, "%s (%d)", str, err);
}
