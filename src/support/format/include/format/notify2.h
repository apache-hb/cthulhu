#pragma once

#include "format/format.h"

#include <stdbool.h>

BEGIN_API

typedef struct event_t event_t;
typedef struct typevec_t typevec_t;

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

typedef struct print_notify_t
{
    print_options_t options;

    heading_style_t heading_style;
    notify_style_t notify_style;

    bool zero_indexed_lines;
} print_notify_t;

void print_notify(print_notify_t config, const event_t *event);
void print_notify_many(print_notify_t config, const typevec_t *events);

END_API
