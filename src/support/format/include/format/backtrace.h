#pragma once

#include "core/analyze.h"

#include "format/format.h"

#include <stdbool.h>

BEGIN_API

typedef struct frame_t frame_t;
typedef struct arena_t arena_t;
typedef struct typevec_t typevec_t;

typedef struct print_backtrace_t
{
    /// @brief basic print options
    print_options_t options;

    /// @brief
    heading_style_t heading_style;

    /// @brief is the first line of the file line 0 or line 1
    bool zero_indexed_lines;
} print_backtrace_t;

/// @brief a backtrace report context
typedef struct bt_report_t bt_report_t;

/// @brief create a new backtrace report
///
/// @return the new backtrace report
bt_report_t *bt_report_new(arena_t *arena);

/// @brief collect a backtrace into a report
/// this is equivalent to calling bt_report_new() and then bt_report_add()
///
/// @return the new backtrace report
bt_report_t *bt_report_collect(arena_t *arena);

/// @brief add a frame to a backtrace report
///
/// @param report the report to add the frame to
/// @param frame the frame to add to the report
void bt_report_add(IN_NOTNULL bt_report_t *report, IN_NOTNULL const frame_t *frame);

/// @brief print a backtrace report
///
/// @param config the configuration to use
/// @param report the report to print
void print_backtrace(print_backtrace_t config, IN_NOTNULL bt_report_t *report);

END_API
