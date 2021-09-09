#pragma once

#include "ctu/util/io.h"

#include <stdint.h>
#include <stddef.h>

typedef int64_t line_t;
typedef int64_t column_t;

typedef struct {
    /* the language name */
    const char *language;

    /* path to the file */
    const char *path;

    /* whatever the file creates by parsing */
    void *data;

    /* the source text of the file */
    const char *text;
    
    /* how much of the text has been read */
    size_t offset;

    /* the length of the text */
    size_t size;

    /* actually reports_t * but forward declaration pains */
    void *reports;
} scan_t;

/* a location inside a scanner */
typedef struct {
    line_t first_line;
    line_t last_line;

    column_t first_column;
    column_t last_column;
} where_t;

extern where_t nowhere;
