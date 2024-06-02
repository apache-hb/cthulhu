// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"
#include "os_common.h"

#include <stdlib.h>
#include <crtdbg.h>

#include "core/win32.h" // IWYU pragma: keep

CT_LOCAL size_t impl_maxname(void)
{
    return MAX_PATH;
}

CT_LOCAL size_t impl_maxpath(void)
{
    return MAX_PATH;
}

CT_LOCAL void impl_init(void)
{
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
}

CT_LOCAL void impl_exit(os_exitcode_t code)
{
    TerminateProcess(GetCurrentProcess(), code);
}

CT_LOCAL void impl_thread_exit(os_status_t status)
{
    ExitThread(status);
    CT_UNREACHABLE();
}

CT_LOCAL void impl_abort(void)
{
    abort();
}
