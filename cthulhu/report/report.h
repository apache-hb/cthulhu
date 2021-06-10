#pragma once

#include <stdint.h>

uint64_t errors(void);

void report(const char *fmt);
void reportf(const char *fmt, ...);
