#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void *ctu_malloc(size_t size) {
    return malloc(size);
}

void *ctu_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void ctu_free(void *ptr) {
    free(ptr);
}
