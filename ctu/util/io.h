#pragma once

#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    const char *path;
    FILE *file;
} file_t;

file_t *ctu_open(const char *path, const char *mode);
void ctu_close(file_t *fp);
bool ctu_valid(const file_t *fp);
