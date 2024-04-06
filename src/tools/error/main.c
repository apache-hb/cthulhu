// SPDX-License-Identifier: GPL-3.0-only

#include "base/panic.h"
#include "setup/setup.h"

int main(void)
{
    setup_global();

    CT_NEVER("error");
}
