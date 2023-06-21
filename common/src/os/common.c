#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

os_result_t *os_result_new(os_error_t error, const void *data, size_t size)
{
    CTASSERT(data != NULL);
    CTASSERT(size > 0);

    os_result_t *result = ctu_malloc(sizeof(os_result_t));
    result->error = error;

    memcpy(result->data, data, size);

    return result;
}

os_error_t os_result_error(os_result_t *result)
{
    CTASSERT(result != NULL);

    return result->error;
}

void *os_result_value(os_result_t *result)
{
    CTASSERT(result != NULL);

    return result->data;
}
