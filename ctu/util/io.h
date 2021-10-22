#pragma once

#include "macros.h"
#include "util.h"

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#define PATH_SEP "/"

typedef enum {
    FILE_READ,
    FILE_WRITE
} file_mode_t;

typedef struct {
    const char *base;
    const char *relative;
    const char *ext;
} path_t;

typedef struct {
    path_t *path;
    FILE *file;
} file_t;

path_t *new_path(const char *base, const char *relative, const char *ext) NONULL;
path_t *relative_path(path_t *base, const char *path) NONULL;
path_t *root_path(void) NONULL;
char *path_realpath(const path_t *path) NONULL;
char *path_relative(const path_t *path) NONULL;
char *path_noext(const path_t *path) NONULL;
char *path_string(const path_t *path) NONULL;
bool path_exists(const path_t *path) NONULL;
file_t *path_open(path_t *path, file_mode_t mode) NONULL;
vector_t *path_parts(const path_t *path) NONULL;

size_t file_read(void *dst, size_t total, file_t *fp) NONULL;
size_t file_size(file_t *fp) NONULL;
void *file_map(file_t *fp) NONULL;
