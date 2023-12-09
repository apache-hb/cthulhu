#include "os_common.h"

#include "memory/memory.h"

#include "base/panic.h"

#include "std/str.h"

#include <string.h>

os_result_t *os_result_new(os_error_t error, const void *data, size_t size)
{
    os_result_t *result = MEM_ALLOC(sizeof(os_result_t) + size, "os_result", NULL);
    result->error = error;

    if (size > 0)
    {
        CTASSERT(data != NULL);
        memcpy(result->data, data, size);
    }

    return result;
}

USE_DECL
os_error_t os_error(os_result_t *result)
{
    CTASSERT(result != NULL);

    return result->error;
}

USE_DECL
void *os_value(os_result_t *result)
{
    CTASSERT(result != NULL);

    return result->data;
}

bool is_special(const char *path)
{
    return path == NULL
        || str_equal(path, ".")
        || str_equal(path, "..");
}
