#pragma once

#include "macros.h"

#include <stdio.h>
#include <stdbool.h>

FILE *compat_fopen(const char *path, const char *mode);
bool compat_file_exists(const char *path);
char *compat_realpath(const char *path);