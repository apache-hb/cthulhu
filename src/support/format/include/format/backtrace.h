// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_format_api.h>

#include "backtrace/backtrace.h"

#include "core/analyze.h"

#include "format/format.h"

#include <stdbool.h>

CT_BEGIN_API

typedef struct arena_t arena_t;
typedef struct typevec_t typevec_t;

/// @defgroup format_backtrace Backtrace pretty printing
/// @ingroup format
/// @brief Pretty printing of backtraces
/// @{

/// @brief a backtrace report context
typedef struct bt_report_t bt_report_t;

/// @brief printing options for a backtrace
typedef struct print_backtrace_t
{
    /// @brief basic print options
    print_options_t options;

    /// @brief the line heading style
    FIELD_RANGE(<, eHeadingCount) heading_style_t heading_style;

    /// @brief is the first line of the file line 0 or line 1
    bool zero_indexed_lines;

    /// @brief path to the root of the projects source directory
    /// if this is set then paths that are under this directory will be trimmed
    /// to be relative to this directory. all other paths remain unchanged
    const char *project_source_path;
} print_backtrace_t;

/// @brief create a new backtrace report
///
/// @return the new backtrace report
CT_FORMAT_API bt_report_t *bt_report_new(IN_NOTNULL arena_t *arena);

/// @brief collect a backtrace into a report
/// this is equivalent to calling bt_report_new() and then bt_report_add()
///
/// @return the new backtrace report
CT_FORMAT_API bt_report_t *bt_report_collect(IN_NOTNULL arena_t *arena);

/// @brief add a frame to a backtrace report
///
/// @param report the report to add the frame to
/// @param frame the frame to add to the report
CT_FORMAT_API void bt_report_add(IN_NOTNULL bt_report_t *report, bt_address_t frame);

/// @brief print a backtrace report
///
/// @param print the configuration to use
/// @param report the report to print
CT_FORMAT_API void print_backtrace(print_backtrace_t print, IN_NOTNULL bt_report_t *report);

/// @}

CT_END_API
