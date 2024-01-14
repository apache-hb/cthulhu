#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include "notify/diagnostic.h"

#include <stdarg.h>
#include <stdbool.h>

BEGIN_API

typedef struct node_t node_t;
typedef struct typevec_t typevec_t;
typedef struct vector_t vector_t;
typedef struct arena_t arena_t;
typedef struct set_t set_t;

/// @defgroup notify Compiler message notification
/// @ingroup common
/// @brief Compiler message logging and error registration
/// @{

typedef struct logger_t logger_t;
typedef struct event_t event_t;
typedef struct segment_t segment_t;

typedef struct event_t
{
    // related diagnostic
    const diagnostic_t *diagnostic;

    // the primary node that this event is attached to
    const node_t *node;
    FIELD_STRING char *message;

    // extra nodes that this event is attached to
    typevec_t *segments;

    // notes attached to this event
    vector_t *notes;
} event_t;

typedef struct segment_t
{
    const node_t *node;
    FIELD_STRING char *message;
} segment_t;

typedef struct event_builder_t
{
    event_t *event;
    arena_t *arena;
} event_builder_t;

typedef struct notify_rules_t
{
    set_t *warnings_as_errors;
    set_t *ignored_warnings;
} notify_rules_t;

/// @brief create a new logger
///
/// @return the new logger
RET_NOTNULL NODISCARD
logger_t *logger_new(IN_NOTNULL arena_t *arena);

/// @brief get the events from the logger
///
/// @param logs the logger
///
/// @return the events
RET_NOTNULL NODISCARD
typevec_t *logger_get_events(IN_NOTNULL const logger_t *logs);

/// @brief check if the logger has any fatal errors
///
/// @param logs the logger
/// @param rules the rules to use
///
/// @return true if the logger has fatal errors
NODISCARD
bool logger_has_errors(IN_NOTNULL const logger_t *logs, notify_rules_t rules);

/// @brief reset the loggers messages
/// @warning this invalidates the events returned by @a logger_get_events
///
/// @param logs the logger
void logger_reset(IN_NOTNULL logger_t *logs);

/// @brief notify the logger of a new message
/// @warning adding a new message invalidates all previous events handles
///
/// @param logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
///
/// @return the new event builder
CT_PRINTF(4, 5)
event_builder_t msg_notify(
    IN_NOTNULL logger_t *logs,
    const diagnostic_t *diagnostic,
    const node_t *node,
    FMT_STRING const char *fmt, ...);

/// @brief notify the logger of a new message
///
/// @param logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param args the format arguments
///
/// @return the new event
event_builder_t msg_vnotify(
    IN_NOTNULL logger_t *logs,
    IN_NOTNULL const diagnostic_t *diagnostic,
    const node_t *node,
    const char *fmt, va_list args);

/// @brief append additional information to a message
///
/// @param builder the event builder to append to
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
CT_PRINTF(3, 4)
void msg_append(
    event_builder_t builder,
    const node_t *node,
    FMT_STRING const char *fmt, ...);

/// @brief append additional information to a message
///
/// @param builder the event builder to append to
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param args the format arguments
void msg_vappend(
    event_builder_t builder,
    const node_t *node,
    const char *fmt, va_list args);

/// @brief add a note to an existing message
///
/// @param builder the event builder to append to
/// @param fmt the format string
/// @param ... the format arguments
CT_PRINTF(2, 3)
void msg_note(
    event_builder_t builder,
    FMT_STRING const char *fmt, ...);

/// @brief add a note to an existing message
///
/// @param builder the event builder to append to
/// @param fmt the format string
/// @param args the format arguments
void msg_vnote(
    event_builder_t builder,
    const char *fmt, va_list args);

/// @}

END_API
