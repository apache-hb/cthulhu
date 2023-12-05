#pragma once

#include "core/compiler.h"

#include "memory/arena.h"

#include <stdarg.h>

BEGIN_API

typedef struct node_t node_t;

typedef struct logger_t logger_t;
typedef struct sink_t sink_t;
typedef struct event_t event_t;

typedef enum severity_t
{
#include "notify/notify.inc"

    eSeverityTotal
} severity_t;

typedef struct diagnostic_t
{
    severity_t severity;
    const char *id;

    const char *brief;
    const char *description;
} diagnostic_t;

typedef struct report_t
{
    const diagnostic_t *diagnostic;
    const node_t *node;

    char *message;
    char *note;
    char *underline;
} report_t;

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

/// @brief create a new logger
///
/// @param alloc the allocator to use
///
/// @return the new logger
logger_t *log_new(alloc_t *alloc);

/// @brief destroy a logger
///
/// @param logs the logger to destroy
void log_delete(OUT_PTR_INVALID logger_t *logs);

/// @brief flush any pending logs
///
/// @param logs the logger
///
/// @return the exit code
int log_flush(logger_t *logs);

/// @brief register a new diagnostic
///
/// @param logs the logger
/// @param diagnostic the diagnostic to register
void msg_diagnostic(logger_t *logs, const diagnostic_t *diagnostic);

/// @brief notify the logger of a new message
/// this is the same as @ref msg_notify but with an attached stacktrace
///
/// @param logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
///
/// @return the new event
event_t *msg_panic(logger_t *logs, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...);

event_t *msg_notify(logger_t *logs, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...);
event_t *msg_vnotify(logger_t *logs, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, va_list args);

void msg_append(event_t *event, const node_t *node, const char *fmt, ...);
void msg_underline(event_t *event, const node_t *node, const char *fmt, ...);
void msg_note(event_t *event, const char *fmt, ...);

END_API
