#pragma once

#include "macros.h"

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    const char *path;
    FILE *file;
} file_t;

file_t *ctu_open(const char *path, const char *mode) NONULL;
void ctu_close(file_t *fp) NONULL;

size_t ctu_read(void *dst, size_t total, file_t *fp) NONULL;
void *ctu_mmap(file_t *fp) NONULL;
