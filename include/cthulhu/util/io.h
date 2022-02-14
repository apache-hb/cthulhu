#pragma once

#include "macros.h"
#include "util.h"

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#ifdef CTU_WINDOWS
#   define PATH_SEP "\\"
#else
#   define PATH_SEP "/"
#endif

typedef struct {
    const char *path;
    FILE *file;
} file_t;

void ctu_close(file_t *fp) NONULL;
file_t ctu_fopen(const char *path, const char *mode) NONULL;
bool file_valid(file_t *fp) CONSTFN NONULL;

size_t ctu_read(void *dst, size_t total, file_t *fp) NONULL;
void *ctu_mmap(file_t *fp) NONULL;

char *ctu_basepath(const char *path) NONULL;
char *ctu_noext(const char *path) NONULL;
char *ctu_filename(const char *path) NONULL;
char *ctu_pathjoin(const char *path, const char *file) NONULL;
