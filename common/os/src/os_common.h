#pragma once

#include "os/error.h"

#include <stdbool.h>

typedef struct os_result_t
{
    os_error_t error;

    char data[];
} os_result_t;

os_result_t *os_result_new(os_error_t error, const void *data, size_t size);

bool is_special(const char *path);
