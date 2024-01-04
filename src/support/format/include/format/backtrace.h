#pragma once

#include "core/analyze.h"
#include "core/compiler.h"
#include "format/notify.h"

BEGIN_API

typedef struct frame_t frame_t;
typedef struct arena_t arena_t;
typedef struct typevec_t typevec_t;

/// @brief a backtrace report context
/// @warning this is an internal structure and should not be used directly
typedef struct bt_report_t
{
    /// @brief memory pool
    arena_t *arena;

    /// @brief all entries
    /// @note sequentially recursive frames are combined into a single entry
    typevec_t *entries;

    /// @brief the longest symbol name
    size_t longest_symbol;

    /// @brief the largest number of consecutive frames
    size_t max_consecutive_frames;

    /// @brief the total number of frames consumed
    /// this is not the same as the number of entries as it includes recursed frames
    size_t total_frames;

    /// @brief the index of the last consecutive frame
    size_t last_consecutive_index;
} bt_report_t;

/// @brief create a new backtrace report
///
/// @return the new backtrace report
bt_report_t bt_report_new(arena_t *arena);

/// @brief collect a backtrace into a report
/// this is equivalent to calling bt_report_new() and then bt_report_add()
///
/// @return the new backtrace report
bt_report_t bt_report_collect(arena_t *arena);

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
