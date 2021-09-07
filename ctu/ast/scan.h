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

    /* the error reporting sink */
    struct reports_t *reports;
} scan_t;

/* create a scanner from a string */
scan_t *scan_string(struct reports_t *reports, const char *language, const char *path, const char *text);

/* create a scanner from a file */
scan_t *scan_file(struct reports_t *reports, const char *language, file_t *file);

/* set the export data */
void scan_export(scan_t *scan, void *data);

typedef struct {
    int(*init)(scan_t *extra, void *scanner);
    void(*set_in)(FILE *fd, void *scanner);
    int(*parse)(void *scanner, void *extra);
    void*(*scan)(const char *text, void *scanner);
    void(*destroy)(void *scanner);
} callbacks_t;

void *compile_string(scan_t *extra, callbacks_t *callbacks);
void *compile_file(scan_t *extra, callbacks_t *callbacks);

/* a location inside a scanner */
typedef struct {
    line_t first_line;
    line_t last_line;

    column_t first_column;
    column_t last_column;
} where_t;

extern where_t nowhere;
