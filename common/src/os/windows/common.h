#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "os/common.h"

os_result_t *win_result(DWORD error, const void *value, size_t size);
os_result_t *win_error(DWORD error);
