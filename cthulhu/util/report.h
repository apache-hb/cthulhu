#pragma once

#include <stdint.h>

#include "util.h"

CTU_API void reportf(const char *fmt, ...);

CTU_API void check_errors(const char *stage);

#define ASSERT(expr) if (!(expr)) { reportf("assert " #expr); exit(1); }
