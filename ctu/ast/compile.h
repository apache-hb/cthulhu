#pragma once

#include "ctu/util/report.h"

#include "scan.h"

/* create a scanner from a string */
scan_t *scan_string(reports_t *reports, size_t ast, const char *language, const char *path, const char *text);

/* create a scanner from a file */
scan_t *scan_file(reports_t *reports, size_t ast, const char *language, file_t *file);

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
