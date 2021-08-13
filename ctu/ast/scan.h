#pragma once

#include "ctu/util/io.h"

#include <stdint.h>
#include <stddef.h>

typedef int64_t line_t;
typedef int64_t column_t;

typedef struct {
    const char *path;
    void *data;

    const char *text;
    size_t offset;
    size_t size;
} scan_t;

scan_t *scan_string(const char *path, const char *text);
scan_t *scan_file(const char *path, FILE *fd);

typedef struct {
    line_t first_line;
    line_t last_line;

    column_t first_column;
    column_t last_column;
} where_t;

extern where_t nowhere;
