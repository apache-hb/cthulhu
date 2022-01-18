#pragma once

#include "cthulhu/util/macros.h"

#include <stdio.h>
#include <stdbool.h>

FILE *compat_fopen(const char *path, const char *mode);
bool compat_file_exists(const char *path);
