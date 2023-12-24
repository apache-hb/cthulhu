#pragma once

#include "os_common.h"

os_result_t *linux_result(int error, const void *value, size_t size);
os_result_t *linux_error(int error);
