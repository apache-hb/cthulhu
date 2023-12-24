#include "os/os.h"

#include <crtdbg.h>
#include <stdlib.h>

void os_init(void)
{
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
}
