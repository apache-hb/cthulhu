#pragma once

#include "core/compiler.h"

#include "notify/notify.h"

BEGIN_API

#define CTU_EVENT(name, ...) extern const diagnostic_t kEvent_##name;
#include "events.inc"

void evt_scan_error(logger_t *logger, node_t *node, const char *msg);
void evt_scan_unknown(logger_t *logger, node_t *node, const char *msg);

event_t *evt_symbol_shadowed(logger_t *logger, const char *name, const node_t *prev,
                             const node_t *next);

END_API
