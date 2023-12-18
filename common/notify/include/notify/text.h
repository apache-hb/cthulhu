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

typedef struct text_colour_t text_colour_t;

BEGIN_API

/// @brief the configuration for a file
typedef struct file_config_t
{
    /// @brief the zeroth line of a file is the first line
    bool zeroth_line;
} file_config_t;

typedef struct text_config_t
{
    // is the first line of a source file the zeroth line or the first line
    file_config_t config;

    const text_colour_t *colours;

    // the io object to write to
    io_t *io;
} text_config_t;

// complex reporting
void text_report_rich(
    text_config_t config,
    IN_NOTNULL const event_t *event);

// simple reporting like msvc
// path(line): severity id: main message

void text_report_simple(
    text_config_t config,
    IN_NOTNULL const event_t *event);

// backtrace reporting

typedef struct bt_report_t bt_report_t;

/// @brief create a new backtrace report
///
/// @return the new backtrace report
bt_report_t *bt_report_new(void);

/// @brief collect a backtrace into a report
/// this is equivalent to calling bt_report_new() and then bt_report_add()
///
/// @return the new backtrace report
bt_report_t *bt_report_collect(void);

/// @brief add a frame to a backtrace report
///
/// @param report the report to add the frame to
/// @param frame the frame to add to the report
void bt_report_add(
    IN_NOTNULL bt_report_t *report,
    IN_NOTNULL const frame_t *frame);

/// @brief print a backtrace report
///
/// @param config the configuration to use
/// @param report the report to print
void bt_report_finish(
    text_config_t config,
    IN_NOTNULL bt_report_t *report);

END_API
