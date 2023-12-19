#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdarg.h>

BEGIN_API

typedef struct node_t node_t;
typedef struct typevec_t typevec_t;
typedef struct vector_t vector_t;
typedef struct arena_t arena_t;

typedef struct logger_t logger_t;
typedef struct event_t event_t;
typedef struct segment_t segment_t;

/// @defgroup Notify Compiler message notification
/// @ingroup Common
/// @brief Compiler message logging and error registration
/// @{

typedef enum severity_t
{
#define SEVERITY(ID, NAME) ID,
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

typedef struct event_t
{
    // related diagnostic
    const diagnostic_t *diagnostic;

    // the primary node that this event is attached to
    const node_t *node;
    char *message;

    // extra nodes that this event is attached to
    typevec_t *segments;

    // notes attached to this event
    vector_t *notes;
} event_t;

typedef struct segment_t
{
    const node_t *node;
    char *message;
} segment_t;

/// @brief create a new logger
///
/// @return the new logger
RET_NOTNULL
NODISCARD
logger_t *logger_new(IN_NOTNULL arena_t *arena);

vector_t *logger_get_events(IN_NOTNULL logger_t *logs);

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
RET_NOTNULL
event_t *msg_panic(
    IN_NOTNULL logger_t *logs,
    IN_NOTNULL const diagnostic_t *diagnostic,
    const node_t *node,
    FMT_STRING const char *fmt, ...) CT_PRINTF(4, 5);

/// @brief notify the logger of a new message
///
/// @param[in, out] logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
///
/// @return the new event
RET_NOTNULL
event_t *msg_notify(
    IN_NOTNULL logger_t *logs,
    const diagnostic_t *diagnostic,
    const node_t *node,
    const char *fmt, ...) CT_PRINTF(4, 5);

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
void msg_append(
    IN_NOTNULL event_t *event,
    const node_t *node,
    FMT_STRING const char *fmt, ...) CT_PRINTF(3, 4);

void msg_vappend(
    IN_NOTNULL event_t *event,
    const node_t *node,
    const char *fmt, va_list args);

/// @brief add a note to an existing message
///
/// @param[in, out] event the event to append to
/// @param fmt the format string
/// @param ... the format arguments
void msg_note(
    IN_NOTNULL event_t *event,
    FMT_STRING const char *fmt, ...) CT_PRINTF(2, 3);

void msg_vnote(
    IN_NOTNULL event_t *event,
    const char *fmt, va_list args);

/// @}

END_API
