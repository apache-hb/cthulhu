#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include "notify/format.h"

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
typedef struct event_t event_t;

BEGIN_API

typedef struct text_config_t
{
    // is the first line of a source file the zeroth line or the first line
    file_config_t config;

    text_colour_t colours;

    // the io object to write to
    io_t *io;
} text_config_t;

void text_report_rich(
    text_config_t config,
    IN_NOTNULL const event_t *event);

void text_report_simple(
    text_config_t config,
    IN_NOTNULL const event_t *event);

END_API
