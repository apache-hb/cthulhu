// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"
#include "os_common.h"

#include <stdlib.h>
#include <crtdbg.h>

#include "core/win32.h" // IWYU pragma: keep

#if CTU_WIN32_TRICKERY
#   include <windef.h>
#   include <winbase.h>
#   include <pathcch.h>
#endif

size_t impl_maxname(void)
{
    return MAX_PATH;
}

size_t impl_maxpath(void)
{
    return MAX_PATH;
}

void os_init(void)
{
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
}

CT_NORETURN os_exit(os_exit_t code)
{
    TerminateProcess(GetCurrentProcess(), code);
    CT_UNREACHABLE();
}
