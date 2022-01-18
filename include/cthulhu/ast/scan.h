#pragma once

#include "cthulhu/util/io.h"
#include "cthulhu/util/str.h"

#include <stdint.h>
#include <stddef.h>

typedef int64_t line_t;
typedef int64_t column_t;

typedef struct {
    size_t size;
    const char *text;
} text_t;

typedef struct {
    /* the language name */
    const char *language;

    /* path to the file */
    const char *path;

    /* whatever the file creates by parsing */
    void *data;

    /* our source text */
    text_t source;

    /* how much of the text has been read */
    size_t offset;

    /* actually reports_t * but forward declaration pains */
    void *reports;
} scan_t;

size_t scan_size(const scan_t *scan);
const char *scan_text(const scan_t *scan);

/* a location inside a scanner */
typedef struct {
    line_t first_line;
    line_t last_line;

    column_t first_column;
    column_t last_column;
} where_t;

extern const where_t nowhere;
