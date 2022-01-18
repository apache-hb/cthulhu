#pragma once

#include "cthulhu/util/report.h"

#include "scan.h"

/* create a scanner from a string */
scan_t scan_string(reports_t *reports, const char *language, const char *path, const char *text);

/* create a scanner from a file */
scan_t scan_file(reports_t *reports, const char *language, file_t *file);

/* set the export data */
void scan_export(scan_t *scan, void *data);

typedef struct {
    // yylex_init_extra
    int(*init)(scan_t *extra, void *scanner);

    // yyset_in
    void(*set_in)(FILE *fd, void *scanner);

    // yyparse
    int(*parse)(scan_t *extra, void *scanner);

    //yy_scan_string
    void*(*scan)(const char *text, void *scanner);

    // yylex_destroy
    void(*destroy)(void *scanner);
} callbacks_t;

/**
 * @brief compile a string into a language specific ast
 * 
 * @param extra the sanner being used
 * @param callbacks the flex/bison callbacks
 * @return void* a pointer to the compiled ast
 */
void *compile_string(scan_t *extra, callbacks_t *callbacks);

/**
 * @brief compile a file to a language specific ast
 * 
 * @param extra the scanner being used
 * @param callbacks the flex/bison callbacks
 * @return void* a pointer to the compiled ast
 */
void *compile_file(scan_t *extra, callbacks_t *callbacks);
