#include "cthulhu/util/error.h"
#include "cthulhu/util/str.h"

#include <string.h>

char *ctu_err_string(int err) {
    char buf[256];
    int result = strerror_r(err, buf, sizeof(buf));

    if (result != 0) {
        return format("unknown error %d", err);
    }

    return ctu_strdup(buf);
}
