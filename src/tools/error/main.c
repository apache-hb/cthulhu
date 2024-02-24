// SPDX-License-Identifier: GPL-3.0-only

#include "backtrace/backtrace.h"
#include "base/panic.h"
#include "setup/setup.h"

int main(void)
{
    setup_global();

    //CTASSERT(false);

    volatile int *ptr = NULL;
    *ptr = 0;
}
