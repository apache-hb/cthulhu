// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_format_api.h>

#include "format/format.h"
#include "core/analyze.h"

#include <stdbool.h>

CT_BEGIN_API

typedef struct event_t event_t;
typedef struct typevec_t typevec_t;

/// @defgroup format_notify Format compiler messages for printing
/// @ingroup format
/// @brief Format compiler messages for printing
/// @{

/// @brief notification style
typedef enum notify_style_t
{
    /// @brief print a single line for each event
    /// just the location and the message
    eNotifyBrief,

    /// @brief print a more full event
    /// includes source text if as well as the location and message
    eNotifyFull,

    eNotifyCount
} notify_style_t;

/// @brief notification formatting options
typedef struct print_notify_t
{
    /// @brief base print options
    print_options_t options;

    /// @brief the style of heading to use
    FIELD_RANGE(<, eHeadingCount) heading_style_t heading_style;

    /// @brief the style of notification to use
    notify_style_t notify_style;

    /// @brief is the first line in a file 0 or 1
    bool zero_indexed_lines;
} print_notify_t;

/// @brief format a single event for printing
///
/// @param config the config to use when printing
/// @param event the event to print
CT_FORMAT_API void print_notify(print_notify_t config, IN_NOTNULL const event_t *event);

/// @brief format many events for printing
///
/// @param config the config to use when printing
/// @param events the events to print
CT_FORMAT_API void print_notify_many(print_notify_t config, IN_NOTNULL const typevec_t *events);

/// @}

CT_END_API
