#pragma once

#include <stdarg.h>
#include <stdbool.h>

#include "util.h"

#define COLOUR_RED "\x1B[1;31m"
#define COLOUR_GREEN "\x1B[1;32m"
#define COLOUR_YELLOW "\x1B[1;33m"
#define COLOUR_BLUE "\x1B[1;34m"
#define COLOUR_PURPLE "\x1B[1;35m"
#define COLOUR_CYAN "\x1B[1;36m"
#define COLOUR_RESET "\x1B[0m"

char *format(const char *fmt, ...);
char *formatv(const char *fmt, va_list args);

bool startswith(const char *str, const char *prefix);
bool endswith(const char *str, const char *suffix);

char *strjoin(const char *sep, vector_t *parts);
char *strmul(const char *str, size_t times);
