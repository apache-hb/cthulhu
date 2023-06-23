#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "os/common.h"

#include <stdint.h>

// pinky promise that these are the same
#define DWORD_MAX UINT32_MAX

os_result_t *win_result(DWORD error, const void *value, size_t size);
os_result_t *win_error(DWORD error);
