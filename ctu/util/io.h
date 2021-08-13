#pragma once

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>

FILE *ctu_open(const char *path, const char *mode);
void ctu_close(FILE *fp);
