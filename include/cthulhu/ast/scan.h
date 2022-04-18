#pragma once

#include "cthulhu/util/io.h"
#include "cthulhu/util/str.h"

#include <stdint.h>
#include <stddef.h>

typedef int64_t line_t;
typedef int64_t column_t;

/**
 * @brief source text
 */
typedef struct {
    size_t size; ///< the number of bytes in the text
    const char *text; ///< the text itself
} text_t;

struct reports_t;

/**
 * @brief a source file
 */
typedef struct {
    const char *language;       ///< the language this file contains
    const char *path;           ///< the path to this file
    void *data;                 ///< user data pointer
    text_t source;              ///< the source text in this file
    size_t offset;              ///< how much of this file has been parsed
    struct reports_t *reports;  ///< the reporting sink for this file
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
