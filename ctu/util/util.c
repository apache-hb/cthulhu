#include "util.h"

#include <stdlib.h>
#include <string.h>

void *copyof(void *ptr, size_t size) {
    void *out = malloc(size);
    memcpy(out, ptr, size);
    return out;
}
