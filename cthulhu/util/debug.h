#pragma once

#include "util.h"

#include <stdio.h>
#include <stddef.h>

CTU_API FILE *get_debug(void);
CTU_API void set_debug(FILE *file);
CTU_API void debugf(const char *fmt, ...);
