// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_notify_api.h>

#include "core/analyze.h"
#include "core/compiler.h"

#include "scan/node.h"

#include "notify/diagnostic.h"

#include <stdarg.h>
#include <stdbool.h>

CT_BEGIN_API

typedef struct node_t node_t;
typedef struct typevec_t typevec_t;
typedef struct vector_t vector_t;
typedef struct arena_t arena_t;
typedef struct set_t set_t;

/// @defgroup notify Compiler message notification
/// @ingroup common
/// @brief Compiler message logging and error registration
/// @{

/// @brief a logging sink
typedef struct logger_t logger_t;

/// @brief an event handle
/// TODO: make this and segment_t opaque
typedef struct event_t
{
    /// @brief the related diagnostic
    const diagnostic_t *diagnostic;

    /// @brief the primary node that this event is attached to
    node_t node;

    /// @brief the primary message
    STA_FIELD_STRING char *message;

    /// @brief extra segments that this event is attached to
    /// @note typevec_t<segment_t>
    typevec_t *segments;

    /// @brief extra notes that this event is attached to
    /// @note vector_t<char*>
    vector_t *notes;
} event_t;

/// @brief a segment inside an event
typedef struct segment_t
{
    /// @brief the related node
    node_t node;

    /// @brief the message associated with this segment
    STA_FIELD_STRING char *message;
} segment_t;

/// @brief an event builder
/// handles adding additional information to an event
typedef struct event_builder_t
{
    /// @brief the event to append to
    event_t *event;

    /// @brief allocation context
    arena_t *arena;
} event_builder_t;

/// @brief a set of rules for filtering notifications
typedef struct notify_rules_t
{
    /// @brief the set of warnings to treat as errors
    /// @note set_t<diagnostic_t>
    set_t *warnings_as_errors;

    /// @brief the set of warnings to ignore
    /// this takes precedence over @a warnings_as_errors
    /// @note set_t<diagnostic_t>
    set_t *ignored_warnings;
} notify_rules_t;

/// @brief create a new logger
///
/// @return the new logger
RET_NOTNULL CT_NODISCARD
CT_NOTIFY_API logger_t *logger_new(IN_NOTNULL arena_t *arena);

/// @brief get the events from the logger
///
/// @param logs the logger
///
/// @return the events
RET_NOTNULL CT_NODISCARD
CT_NOTIFY_API typevec_t *logger_get_events(IN_NOTNULL const logger_t *logs);

/// @brief check if the logger has any fatal errors
///
/// @param logs the logger
/// @param rules the rules to use
///
/// @return true if the logger has fatal errors
CT_NODISCARD
CT_NOTIFY_API bool logger_has_errors(IN_NOTNULL const logger_t *logs, notify_rules_t rules);

/// @brief reset the loggers messages
/// @warning this invalidates the events returned by @a logger_get_events
///
/// @param logs the logger
CT_NOTIFY_API void logger_reset(IN_NOTNULL logger_t *logs);

RET_NOTNULL CT_NODISCARD
CT_NOTIFY_API arena_t *logger_get_arena(IN_NOTNULL const logger_t *logs);

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
STA_PRINTF(4, 5)
CT_NOTIFY_API event_builder_t msg_notify(
    INOUT_NOTNULL logger_t *logs,
    IN_NOTNULL const diagnostic_t *diagnostic,
    const node_t *node,
    STA_FORMAT_STRING const char *fmt, ...);

/// @brief notify the logger of a new message
///
/// @param logs the logger
/// @param diagnostic the diagnostic to use
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param args the format arguments
///
/// @return the new event
CT_NOTIFY_API event_builder_t msg_vnotify(
    INOUT_NOTNULL logger_t *logs,
    IN_NOTNULL const diagnostic_t *diagnostic,
    const node_t *node,
    IN_STRING const char *fmt, va_list args);

/// @brief append additional information to a message
///
/// @param builder the event builder to append to
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param ... the format arguments
STA_PRINTF(3, 4)
CT_NOTIFY_API void msg_append(
    event_builder_t builder,
    const node_t *node,
    STA_FORMAT_STRING const char *fmt, ...);

/// @brief append additional information to a message
///
/// @param builder the event builder to append to
/// @param node the node to attach to the message
/// @param fmt the format string
/// @param args the format arguments
CT_NOTIFY_API void msg_vappend(
    event_builder_t builder,
    const node_t *node,
    IN_STRING const char *fmt, va_list args);

/// @brief add a note to an existing message
///
/// @param builder the event builder to append to
/// @param fmt the format string
/// @param ... the format arguments
STA_PRINTF(2, 3)
CT_NOTIFY_API void msg_note(
    event_builder_t builder,
    STA_FORMAT_STRING const char *fmt, ...);

/// @brief add a note to an existing message
///
/// @param builder the event builder to append to
/// @param fmt the format string
/// @param args the format arguments
CT_NOTIFY_API void msg_vnote(
    event_builder_t builder,
    IN_STRING const char *fmt, va_list args);

/// @}

CT_END_API
