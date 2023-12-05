#include "notify/notify.h"

#include "std/vector.h"

#include "core/macros.h"

typedef struct logger_t
{
    alloc_t *alloc;

    vector_t *messages;
} logger_t;

logger_t *log_new(alloc_t *alloc)
{
    logger_t *logs = arena_malloc(alloc, sizeof(logger_t), "logger", NULL);
    logs->alloc = alloc;
    return logs;
}

void log_delete(logger_t *logs)
{
    CTU_UNUSED(logs);
    // empty for now
}

int log_flush(logger_t *logs)
{
    CTU_UNUSED(logs);

    return 0;
}

void msg_diagnostic(logger_t *logs, const diagnostic_t *diagnostic)
{
    CTU_UNUSED(logs);
    CTU_UNUSED(diagnostic);
}

event_t *msg_panic(logger_t *logs, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    event_t *event = msg_vnotify(logs, diagnostic, node, fmt, args);

    va_end(args);

    return event;
}

event_t *msg_notify(logger_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    event_t *event = msg_vnotify(reports, diagnostic, node, fmt, args);

    va_end(args);

    return event;
}

event_t *msg_vnotify(logger_t *logs, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, va_list args)
{
    CTU_UNUSED(logs);
    CTU_UNUSED(diagnostic);
    CTU_UNUSED(node);
    CTU_UNUSED(fmt);
    CTU_UNUSED(args);

    return NULL;
}

void msg_append(event_t *event, const node_t *node, const char *fmt, ...)
{
    CTU_UNUSED(event);
    CTU_UNUSED(node);
    CTU_UNUSED(fmt);
}

void msg_underline(event_t *event, const node_t *node, const char *fmt, ...)
{
    CTU_UNUSED(event);
    CTU_UNUSED(node);
    CTU_UNUSED(fmt);
}

void msg_note(event_t *event, const char *fmt, ...)
{
    CTU_UNUSED(event);
    CTU_UNUSED(fmt);
}
