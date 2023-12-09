#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdarg.h>

BEGIN_API

typedef struct node_t node_t;
typedef struct vector_t vector_t;
typedef struct arena_t arena_t;

typedef struct logger_t logger_t;
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

typedef struct segment_t
{
    const node_t *node;
    char *message;
} segment_t;

typedef struct report_t
{
    const diagnostic_t *diagnostic;
    const node_t *node;

    char *message;
    char *note;
    char *underline;
} report_t;

/// @brief create a new logger
///
/// @return the new logger
RET_NOTNULL
NODISCARD
logger_t *log_new(void);

/// @brief destroy a logger
///
/// @param logs the logger to destroy
void log_delete(OUT_PTR_INVALID logger_t *logs);

/// @brief get all pending log messages
/// get all log messages from a logger and reset the logger
///
/// @param logs the logger
///
/// @return all pending log messages
RET_NOTNULL
NODISCARD
vector_t *log_reset(IN_NOTNULL logger_t *logs);

/// @brief register a new diagnostic
///
/// @param[in, out] logs the logger
/// @param diagnostic the diagnostic to register
void msg_diagnostic(
    IN_NOTNULL logger_t *logs,
    IN_NOTNULL const diagnostic_t *diagnostic);

/// @brief notify the logger of a new message
/// this is the same as @ref msg_notify but with an attached stacktrace
///
/// @param[in, out] logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
///
/// @return the new event
CT_PRINTF(4, 5)
RET_NOTNULL
event_t *msg_panic(
    IN_NOTNULL logger_t *logs,
    IN_NOTNULL const diagnostic_t *diagnostic,
    const node_t *node,
    FORMAT_STRING const char *fmt, ...);

/// @brief notify the logger of a new message
///
/// @param[in, out] logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
///
/// @return the new event
CT_PRINTF(4, 5)
RET_NOTNULL
event_t *msg_notify(
    IN_NOTNULL logger_t *logs,
    const diagnostic_t *diagnostic,
    const node_t *node,
    const char *fmt, ...);

/// @brief notify the logger of a new message
///
/// @param[in, out] logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param args the format arguments
///
/// @return the new event
RET_NOTNULL
event_t *msg_vnotify(
    IN_NOTNULL logger_t *logs,
    IN_NOTNULL const diagnostic_t *diagnostic,
    const node_t *node,
    const char *fmt, va_list args);

/// @brief append additional information to a message
///
/// @param[in, out] event the event to append to
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
CT_PRINTF(3, 4)
void msg_append(
    IN_NOTNULL event_t *event,
    const node_t *node,
    FORMAT_STRING const char *fmt, ...);

void msg_vappend(
    IN_NOTNULL event_t *event,
    const node_t *node,
    const char *fmt, va_list args);

/// @brief add an underline message and range to an existing message
///
/// @param[in, out] event the event to append to
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
CT_PRINTF(3, 4)
void msg_underline(
    IN_NOTNULL event_t *event,
    const node_t *node,
    FORMAT_STRING const char *fmt, ...);

void msg_vunderline(
    IN_NOTNULL event_t *event,
    const node_t *node,
    const char *fmt, va_list args);

/// @brief add a note to an existing message
///
/// @param[in, out] event the event to append to
/// @param fmt the format string
/// @param ... the format arguments
CT_PRINTF(2, 3)
void msg_note(
    IN_NOTNULL event_t *event,
    FORMAT_STRING const char *fmt, ...);

void msg_vnote(
    IN_NOTNULL event_t *event,
    const char *fmt, va_list args);

END_API
