#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>

/// message without source
///
/// severity [id]: main message
/// note: note message

/// the = in the => arrow is aligned with the | pipe
/// the : in the note: is aligned with the | pipe
/// all line numbers are right aligned
/// all reports from the same file are grouped together
///     * are sorted by line number
///     * pipes in the same file are all aligned together

/// rich message format
///
/// severity [id]: main message
///      => lang [path:line]
///      |
/// line | source code
///      |        ^~~~ underline message
///
///  note: note message

typedef struct io_t io_t;
typedef struct frame_t frame_t;
typedef struct event_t event_t;
typedef struct typevec_t typevec_t;
typedef struct arena_t arena_t;

typedef struct colour_pallete_t colour_pallete_t;
typedef struct cache_map_t cache_map_t;
typedef struct set_t set_t;
typedef struct text_cache_t text_cache_t;

BEGIN_API

/// @brief the configuration for a file
typedef struct file_config_t
{
    /// @brief the zeroth line of a file is the first line
    bool zeroth_line;

    /// @brief the maximum number of columns to print
    /// set to 0 to use default
    size_t max_columns;

    /// @brief if true all warnings are treated as fatal
    bool override_fatal;
} file_config_t;

typedef enum text_format_t
{
    eTextSimple,    ///< simple text reporting
    eTextComplex,   ///< complex text reporting

    eTextTotal
} text_format_t;

typedef struct text_config_t
{
    /// a shared cache between all reports, set to NULL to disable caching
    cache_map_t *cache;

    // is the first line of a source file the zeroth line or the first line
    file_config_t config;

    // colour configuration
    const colour_pallete_t *colours;

    // the io object to write to
    io_t *io;
} text_config_t;

typedef struct report_config_t
{
    size_t max_errors;
    size_t max_warnings;

    set_t *ignore_warnings;
    set_t *error_warnings;

    text_format_t report_format;
    text_config_t text_config;
} report_config_t;

int text_report(
    IN_NOTNULL typevec_t *events,
    report_config_t config,
    IN_STRING const char *title);

// complex reporting
void text_report_rich(
    text_config_t config,
    IN_NOTNULL const event_t *event);

// simple reporting like msvc
// path(line): severity id: main message

void text_report_simple(
    text_config_t config,
    IN_NOTNULL const event_t *event);

END_API
