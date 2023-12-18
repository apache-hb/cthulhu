#include "common.h"

#include <crtdbg.h>

void os_init(void)
{
    _CrtSetReportMode(_CRT_ASSERT, 0);
}

os_result_t *win_result(DWORD error, const void *value, size_t size)
{
    return os_result_new(error, value, size);
}

os_result_t *win_error(DWORD error)
{
    return win_result(error, NULL, 0);
}
