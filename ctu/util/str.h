#pragma once

#include <stdbool.h>
#include <stdarg.h>

#define COLOUR_RED "\x1B[1;31m"
#define COLOUR_YELLOW "\x1B[1;33m"
#define COLOUR_BLUE "\x1B[1;34m"
#define COLOUR_PURPLE "\x1B[1;35m"
#define COLOUR_CYAN "\x1B[1;36m"
#define COLOUR_RESET "\x1B[0m"

bool startswith(const char *str, const char *other);

char *formatv(const char *fmt, va_list args);
char *format(const char *fmt, ...);

char *strdup(const char *str);
