#include "common.h"

#include "base/panic.h"

os_result_t *win_result(DWORD error, const void *value, size_t size)
{
    return os_result_new(error, value, size);
}

os_result_t *win_error(DWORD error)
{
    return win_result(error, NULL, 0);
}
