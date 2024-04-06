// SPDX-License-Identifier: GPL-3.0-only

#include "base/panic.h"
#include "setup/setup2.h"

int main(void)
{
    setup_default(NULL);

    CT_NEVER("error");
}
