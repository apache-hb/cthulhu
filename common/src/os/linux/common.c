#include "common.h"

os_result_t *linux_result(int error, const void *value, size_t size)
{
    return os_result_new(error, value, size);
}

os_result_t *linux_error(int error)
{
    return linux_result(error, NULL, 0);
}
