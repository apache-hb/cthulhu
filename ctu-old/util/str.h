#pragma once

#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#define COLOUR_RED "\x1B[1;31m"
#define COLOUR_GREEN "\x1B[1;32m"
#define COLOUR_YELLOW "\x1B[1;33m"
#define COLOUR_BLUE "\x1B[1;34m"
#define COLOUR_PURPLE "\x1B[1;35m"
#define COLOUR_CYAN "\x1B[1;36m"
#define COLOUR_RESET "\x1B[0m"

bool startswith(const char *str, const char *other);
char *str_replace(const char *str, const char *old, const char *with);
char *str_join(const char *sep, const char **strs, size_t num);
size_t str_find(const char *str, const char *substr);

char *formatv(const char *fmt, va_list args);
char *format(const char *fmt, ...);

char *ctu_strdup(const char *str);
