// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_events_api.h>

#include "core/compiler.h"

#include "notify/notify.h"
#include "os/core.h"

CT_BEGIN_API

/// @defgroup events Compiler events
/// @ingroup runtime
/// @brief Generic errors shared between frontends
/// @{

#define CTU_EVENT(name, ...) CT_EVENTS_API extern const diagnostic_t kEvent_##name;
#include "events.inc"

/// @brief get all common diagnostics
///
/// @return all common diagnostics
CT_NODISCARD CT_CONSTFN
CT_EVENTS_API diagnostic_list_t get_common_diagnostics(void);

/// @brief signal that a scan error has occurred
///
/// @param logger the logger to report to
/// @param node the node that caused the error
/// @param msg the error message
CT_EVENTS_API void evt_scan_error(
    IN_NOTNULL logger_t *logger,
    IN_NOTNULL const node_t *node,
    IN_STRING const char *msg);

/// @brief signal that a scanner has encountered an unknown token
///
/// @param logger the logger to report to
/// @param node the node that caused the error
/// @param msg the error message
CT_EVENTS_API void evt_scan_unknown(
    IN_NOTNULL logger_t *logger,
    IN_NOTNULL const node_t *node,
    IN_STRING const char *msg);

/// @brief signal that a declaration would shadow a previous declaration
///
/// @param logger the logger to report to
/// @param name the name of the symbol
/// @param prev the previous declaration
/// @param next the new declaration
///
/// @return the event builder
CT_EVENTS_API event_builder_t evt_symbol_shadowed(
    IN_NOTNULL logger_t *logger,
    IN_STRING const char *name,
    IN_NOTNULL const node_t *prev,
    IN_NOTNULL const node_t *next);

/// @brief signal that an os error has occurred
///
/// @param logger the logger to report to
/// @param error the os error
/// @param msg the error message
///
/// @return the event builder
CT_EVENTS_API event_builder_t evt_os_error(
    IN_NOTNULL logger_t *logger,
    const diagnostic_t *diagnostic,
    os_error_t error,
    IN_STRING const char *msg);

/// @}

CT_END_API
