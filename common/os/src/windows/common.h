#pragma once

#include "os_common.h"

#include "core/win32.h"

#include <stdint.h>

os_result_t *win_result(DWORD error, const void *value, size_t size);
os_result_t *win_error(DWORD error);
