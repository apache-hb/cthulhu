#pragma once

#include "macros.h"

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#define PATH_SEP "/"

typedef struct {
    const char *path;
    NULLABLE FILE *file;
} file_t;

void ctu_close(OWNED file_t *fp) NONULL;
OWNED file_t *ctu_fopen(const char *path, const char *mode) NONULL ALLOC(ctu_close);

size_t ctu_read(WEAK void *dst, size_t total, file_t *fp) NONULL;
OWNED void *ctu_mmap(WEAK file_t *fp) NONULL;

char *ctu_basepath(const char *path) NONULL;
