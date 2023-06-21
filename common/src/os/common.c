#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

static os_result_t *os_result_new(bool valid, void *data, size_t size)
{
    CTASSERT(data != NULL);
    CTASSERT(size > 0);

    os_result_t *result = ctu_malloc(sizeof(os_result_t));
    result->valid = valid;

    memcpy(result->data, data, size);

    return result;
}

os_result_t *os_error(void *data, size_t size)
{
    return os_result_new(false, data, size);
}

os_result_t *os_value(void *data, size_t size)
{
    return os_result_new(true, data, size);
}

bool os_result_valid(const os_result_t *result)
{
    CTASSERT(result != NULL);

    return result->valid;
}