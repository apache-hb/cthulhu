#include "common.h"

#include "base/panic.h"

#include <objbase.h>

void os_init(void)
{
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
}

os_result_t *win_result(DWORD error, const void *value, size_t size)
{
    return os_result_new(error, value, size);
}

os_result_t *win_error(DWORD error)
{
    return win_result(error, NULL, 0);
}
