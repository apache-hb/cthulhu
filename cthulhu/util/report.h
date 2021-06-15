#pragma once

#include <stdint.h>
#include <stdarg.h>

#include "util.h"

#define COLOUR_RED "\x1B[1;31m"
#define COLOUR_CYAN "\x1B[1;36m"
#define COLOUR_RESET "\x1B[0m"

CTU_API void reportf(const char *fmt, ...);
CTU_API void reportv(const char *fmt, va_list args);

CTU_API bool check_errors(const char *stage);
CTU_API void add_fail(void);

#define ASSERT(expr) if (!(expr)) { reportf("assert " #expr); exit(1); }
