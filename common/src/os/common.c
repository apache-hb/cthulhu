#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

#include "report/report.h"

os_result_t *os_result_new(os_error_t error, const void *data, size_t size)
{
    os_result_t *result = ctu_malloc(sizeof(os_result_t) + size);
    result->error = error;

    if (size > 0)
    {
        CTASSERT(data != NULL);
        memcpy(result->data, data, size);
    }

    logverbose("os_result_new: %p, data = %p, size = %zu, diff = %zu", result, result->data, size, (uintptr_t)result->data - (uintptr_t)result);

    return result;
}

os_error_t os_error(os_result_t *result)
{
    CTASSERT(result != NULL);

    return result->error;
}

void *os_value(os_result_t *result)
{
    CTASSERT(result != NULL);

    return result->data;
}
