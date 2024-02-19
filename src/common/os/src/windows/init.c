#include "os/core.h"

#include <stdlib.h>
#include <crtdbg.h>

#include "core/win32.h" // IWYU pragma: keep

#if CTU_WIN32_TRICKERY
#   include <windef.h>
#   include <winbase.h>
#   include <pathcch.h>
#endif

void os_init(void)
{
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
}
