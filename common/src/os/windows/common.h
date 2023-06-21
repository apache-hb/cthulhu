#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "os/common.h"

typedef union win_result_t
{
    DWORD error;
    void *value;
} win_result_t;

os_result_t *win_error(DWORD error);
os_result_t *win_value(void *value);
