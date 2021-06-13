#pragma once

#include <stdbool.h>

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
