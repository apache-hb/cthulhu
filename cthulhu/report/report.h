#pragma once

#include <stdint.h>

#include "cthulhu/util.h"

CTU_API uint64_t errors(void);

CTU_API void report(const char *fmt);
CTU_API void reportf(const char *fmt, ...);
