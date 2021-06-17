#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * d: data pointer
 * l: length
 * s: size
 * i: sizeof(item)
 * g: growth amount
 */
#define ENSURE_SIZE(d, l, s, i, g) if (l + 1 > s) { s += g; d = realloc(d, i * s); }

#ifdef __cplusplus
#   define CTU_API extern "C"
#else
#   define CTU_API
#endif

bool startswith(const char *str, const char *sub);

char *format(const char *fmt, ...);
char *formatv(const char *fmt, va_list args);

#ifndef __cplusplus
#   ifdef _MSC_VER
#       define strdup _strdup
#   else
char *strdup(const char *str);
#   endif
#endif
