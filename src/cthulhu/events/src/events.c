#include "cthulhu/events/events.h"
#include "scan/node.h"
#include "std/str.h"

/// declare all the events

#define CTU_EVENT(name, ...) const diagnostic_t kEvent_##name = __VA_ARGS__;
#include "cthulhu/events/events.def"

/// create the table of events

static const diagnostic_t * const kDiagnosticTable[] = {
#define CTU_EVENT(name, ...) &kEvent_##name,
#include "cthulhu/events/events.def"
};

#define DIAGNOTSIC_COUNT (sizeof(kDiagnosticTable) / sizeof(diagnostic_t*))

diagnostic_list_t get_common_diagnostics(void)
{
    diagnostic_list_t list = {
        .diagnostics = kDiagnosticTable,
        .count = DIAGNOTSIC_COUNT,
    };

    return list;
}

void evt_scan_error(logger_t *logger, const node_t *node, const char *msg)
{
    msg_notify(logger, &kEvent_ParseFailed, node, "%s", msg);
}

void evt_scan_unknown(logger_t *logger, const node_t *node, const char *msg)
{
    const scan_t *scan = node_get_scan(node);
    arena_t *arena = scan_get_arena(scan);
    msg_notify(logger, &kEvent_UnknownToken, node, "unknown symbol: `%s`", str_normalize(msg, arena));
}

event_builder_t evt_symbol_shadowed(logger_t *logger, const char *name, const node_t *prev,
                             const node_t *next)
{
    event_builder_t event = msg_notify(logger, &kEvent_SymbolShadowed, next, "symbol `%s` shadows previous declaration", name);
    msg_append(event, prev, "previous declaration here");
    return event;
}
