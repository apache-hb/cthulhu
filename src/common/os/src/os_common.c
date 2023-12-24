#include "os_common.h"

#include "std/str.h"

bool is_special(const char *path)
{
    return path == NULL
        || str_equal(path, ".")
        || str_equal(path, "..");
}
