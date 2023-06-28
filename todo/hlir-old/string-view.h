#pragma once

#include <stddef.h>

typedef struct string_view_t {
    const char *data;
    size_t size;
} string_view_t;
