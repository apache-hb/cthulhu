#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>

/// small message format
///
/// severity [id]: main message
///      => lang [path:line:col]
///      |
/// line | source code
///      |        ^~~~ underline message
///  note: note message

/// medium 1 message format
///
/// severity [id]: main message
///       => lang [path:line:col]
///       |
/// line1 > source code
/// line2 > source code
///       ^~~~~~~~~~~~~ underline message
///       |
///   note: note message

/// medium 2 message format
///
/// severity [id]: main message
///       => lang [path:line:col]
///       |
/// line1 > source code
/// line2 > source code
/// line3 > source code
///       ^~~~~~~~~~~~~ underline message
///       |
///   note: note message

/// large message format
///
/// severity [id]: main message
///       => lang [path:line:col]
///       |
/// line1 > source code
///       ...
/// lineN > source code
///       ^~~~~~~~~~~~~ underline message
///       |
///   note: note message

typedef struct io_t io_t;
typedef struct event_t event_t;

BEGIN_API

typedef struct text_config_t
{
    io_t *io;
    bool colour;
} text_config_t;

void text_report(
    text_config_t config,
    IN_NOTNULL const event_t *event);

END_API
