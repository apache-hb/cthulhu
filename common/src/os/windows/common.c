#include "common.h"

os_result_t *win_error(DWORD error)
{
    win_result_t result = { .error = error };
    return os_error(&result, sizeof(win_result_t));
}

os_result_t *win_value(void *value)
{
    win_result_t result = { .value = value };
    return os_value(&result, sizeof(win_result_t));
}
