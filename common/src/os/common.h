#pragma once

#include "os/os.h"

typedef struct os_result_t 
{
    bool valid;

    char data[];
} os_result_t;

os_result_t *os_error(void *data, size_t size);
os_result_t *os_value(void *data, size_t size);
