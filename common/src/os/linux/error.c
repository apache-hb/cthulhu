#include "common.h"

#include "std/str.h"

#include <string.h>

USE_DECL
const char *os_decode(os_error_t error)
{
    int err = error;

    const char *str = strerror(err);
    if (str == NULL)
    {
        return format("errno: %d", err);
    }

    return format("%s (%d)", str, err);
}
