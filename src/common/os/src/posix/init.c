// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"

#include <limits.h>
#include <stdlib.h>

long gMaxPathLength = 0;

void os_init(void)
{
    // empty
    long size = pathconf(".", _PC_PATH_MAX);
    if (size < 0)
    {
        // best guess if pathconf() fails
        // TODO: should there be a way to notify the user that the path length is unknown?
        gMaxPathLength = PATH_MAX;
    }
    else
    {
        gMaxPathLength = size;
    }
}

CT_NORETURN os_exit(os_exit_t code)
{
    exit(code); // NOLINT(concurrency-mt-unsafe)
}
