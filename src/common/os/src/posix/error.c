#include "os/os.h"

#include "std/str.h"

#include <string.h>

USE_DECL
const char *os_error_string(os_error_t error)
{
    int err = (int)error;

    const char *str = strerror(err);
    if (str == NULL)
    {
        return format("errno: %d", err);
    }

    return format("%s (%d)", str, err);
}
