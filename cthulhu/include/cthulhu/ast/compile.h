#pragma once

#include "cthulhu/report/report.h"
#include "platform/file.h"
#include "scan.h"

#include <stdio.h>

/**
 * scanner function callbacks for flex and bison
 */
typedef struct
{
    int (*init)(scan_t *extra, void *scanner);                   ///< yylex_init_extra
    int (*parse)(void *scanner, scan_t *extra);                  ///< yyparse
    void *(*scan)(const char *text, size_t size, void *scanner); ///< yy_scan_string
    void (*destroyBuffer)(void *buffer, void *scanner);          ///< yy_delete_buffer
    void (*destroy)(void *scanner);                              ///< yylex_destroy
} callbacks_t;

/**
 * @brief create a scanner from a source string
 *
 * @param reports the reporting sink
 * @param language the language this file contains
 * @param path the path to the file
 * @param text the source text inside the file
 * @return the populated scanner
 */
scan_t scan_string(reports_t *reports, const char *language, const char *path, const char *text);

/**
 * @brief create a scanner from a file
 *
 * @param reports the reporting sink
 * @param language the language this file contains
 * @param file a file object
 * @return the populated scanner
 */
scan_t scan_file(reports_t *reports, const char *language, file_t file);

scan_t scan_without_source(reports_t *reports, const char *language, const char *path);

/**
 * @brief compile a string into a language specific ast
 *
 * @param extra the sanner being used
 * @param callbacks the flex/bison callbacks
 * @return void* a pointer to the compiled ast
 */
void *compile_string(scan_t *extra, callbacks_t *callbacks);
