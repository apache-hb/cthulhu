#include "cthulhu/events/events.h"
#include "std/str.h"

#define CTU_EVENT(name, ...) const diagnostic_t kEvent_##name = __VA_ARGS__;

#include "cthulhu/events/events.inc"

void evt_scan_error(logger_t *logger, node_t *node, const char *msg)
{
    msg_notify(logger, &kEvent_ParseFailed, node, "scan error: `%s`", msg);
}

void evt_scan_unknown(logger_t *logger, node_t *node, const char *msg)
{
    msg_notify(logger, &kEvent_UnknownToken, node, "unknown symbol: `%s`", str_normalize(msg));
}

event_t *evt_symbol_shadowed(logger_t *logger, const char *name, const node_t *prev,
                             const node_t *next)
{
    event_t *event = msg_notify(logger, &kEvent_SymbolShadowed, next, "symbol `%s` shadows previous declaration", name);
    msg_append(event, prev, "previous declaration here");
    return event;
}
