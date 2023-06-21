#pragma once

#include "os/os.h"

typedef struct os_result_t 
{
    os_error_t error;
    
    char data[];
} os_result_t;

os_result_t *os_result_new(os_error_t error, const void *data, size_t size);
