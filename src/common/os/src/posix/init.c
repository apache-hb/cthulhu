// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"

#include <stdlib.h>

void os_init(void)
{
    // empty
}

CT_NORETURN os_exit(os_exit_t code)
{
    exit(code); // NOLINT(concurrency-mt-unsafe)
}
