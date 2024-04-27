// SPDX-License-Identifier: GPL-3.0-only

#include "base/panic.h"
#include "setup/setup.h"

int main(void)
{
    setup_default(NULL);

    volatile int *a = NULL;
    *a = 0;
    // CT_NEVER("error");
}
