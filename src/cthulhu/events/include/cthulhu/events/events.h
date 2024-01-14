#pragma once

#include "core/compiler.h"

#include "notify/notify.h"

BEGIN_API

/// @defgroup events Compiler events
/// @ingroup runtime
/// @brief Generic errors shared between frontends
/// @{

#define CTU_EVENT(name, ...) extern const diagnostic_t kEvent_##name;
#include "events.inc"

/// @brief get all common diagnostics
///
/// @return all common diagnostics
diagnostic_list_t get_common_diagnostics(void);

/// @brief signal that a scan error has occurred
///
/// @param logger the logger to report to
/// @param node the node that caused the error
/// @param msg the error message
void evt_scan_error(logger_t *logger, const node_t *node, const char *msg);

/// @brief signal that a scanner has encountered an unknown token
///
/// @param logger the logger to report to
/// @param node the node that caused the error
/// @param msg the error message
void evt_scan_unknown(logger_t *logger, const node_t *node, const char *msg);

/// @brief signal that a declaration would shadow a previous declaration
///
/// @param logger the logger to report to
/// @param name the name of the symbol
/// @param prev the previous declaration
/// @param next the new declaration
///
/// @return the event builder
event_builder_t evt_symbol_shadowed(logger_t *logger, const char *name, const node_t *prev,
                             const node_t *next);

/// @}

END_API
