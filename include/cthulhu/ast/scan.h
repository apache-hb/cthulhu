#pragma once

#include "cthulhu/util/io.h"
#include "cthulhu/util/str.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @ingroup LocationTracking
 * @{
 */

typedef int64_t line_t;   ///< line number
typedef int64_t column_t; ///< column number

/**
 * @brief source text
 */
typedef struct
{
    size_t size;      ///< the number of bytes in the text
    const char *text; ///< the text itself
} text_t;

struct reports_t;

/**
 * @brief a source file
 */
typedef struct
{
    const char *language;      ///< the language this file contains
    const char *path;          ///< the path to this file
    void *data;                ///< user data pointer
    text_t source;             ///< the source text in this file
    size_t offset;             ///< how much of this file has been parsed
    struct reports_t *reports; ///< the reporting sink for this file
} scan_t;

/**
 * @brief get the number of characters in a source file
 *
 * @param scan the scanner to get the length of
 * @return the number of characters in the source file
 */
size_t scan_size(const scan_t *scan);

/**
 * @brief get the source text of a source file
 *
 * @param scan the scanner to query
 * @return the null terminated source text
 */
const char *scan_text(const scan_t *scan);

/**
 * @brief a location inside a scanner
 */
typedef struct
{
    line_t firstLine; ///< the first line of the location
    line_t lastLine;  ///< the last line of the location

    column_t firstColumn; ///< the first column of the location
    column_t lastColumn;  ///< the last column of the location
} where_t;

/** @} */
