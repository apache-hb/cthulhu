#include "notify/notify.h"

#include "base/panic.h"
#include "memory/memory.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

typedef struct logger_t
{
    vector_t *diagnostics;
    map_t *lookup;

    vector_t *messages;
} logger_t;

USE_DECL
logger_t *log_new(void)
{
    logger_t *logs = MEM_ALLOC(sizeof(logger_t), "logger", NULL);
    logs->diagnostics = vector_new(64);
    logs->lookup = map_optimal(256);
    logs->messages = vector_new(32);
    return logs;
}

USE_DECL
void log_delete(logger_t *logs)
{
    CTASSERT(logs != NULL);

    ctu_free(logs);
}

USE_DECL
vector_t *log_events(logger_t *logs)
{
    CTASSERT(logs != NULL);

    return logs->messages;
}

USE_DECL
void msg_diagnostic(logger_t *logs, const diagnostic_t *diagnostic)
{
    CTASSERT(logs != NULL);
    CTASSERT(diagnostic != NULL);

    const diagnostic_t *old = map_get(logs->lookup, diagnostic->id);
    CTASSERTF(old == NULL, "diagnostic %s already exists", diagnostic->id);

    vector_push(&logs->diagnostics, (void*)diagnostic);
    map_set(logs->lookup, diagnostic->id, (void*)diagnostic);
}

USE_DECL
event_t *msg_panic(logger_t *logs, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...)
{
    CTASSERT(logs != NULL);
    CTASSERT(diagnostic != NULL);
    CTASSERT(fmt != NULL);

    va_list args;
    va_start(args, fmt);

    event_t *event = msg_vnotify(logs, diagnostic, node, fmt, args);

    va_end(args);

    return event;
}

USE_DECL
event_t *msg_notify(logger_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    event_t *event = msg_vnotify(reports, diagnostic, node, fmt, args);

    va_end(args);

    return event;
}

USE_DECL
event_t *msg_vnotify(logger_t *logs, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, va_list args)
{
    CTASSERT(logs != NULL);
    CTASSERT(diagnostic != NULL);

    CTASSERTF(map_get(logs->lookup, diagnostic->id) != NULL, "diagnostic %s was not registered prior to use", diagnostic->id);

    char *msg = vformat(fmt, args);

    event_t event = {
        .diagnostic = diagnostic,
        .node = node,
        .message = msg
    };

    event_t *ptr = BOX(event);

    vector_push(&logs->messages, ptr);

    return ptr;
}

USE_DECL
void msg_append(event_t *event, const node_t *node, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    msg_vappend(event, node, fmt, args);

    va_end(args);
}

USE_DECL
void msg_vappend(event_t *event, const node_t *node, const char *fmt, va_list args)
{
    CTASSERT(event != NULL);
    CTASSERT(fmt != NULL);

    if (event->segments == NULL)
    {
        event->segments = typevec_new(sizeof(segment_t), 2);
    }

    char *msg = vformat(fmt, args);

    segment_t segment = {
        .node = node,
        .message = msg
    };

    typevec_push(event->segments, &segment);
}

USE_DECL
void msg_underline(event_t *event, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    msg_vunderline(event, fmt, args);

    va_end(args);
}

USE_DECL
void msg_vunderline(event_t *event, const char *fmt, va_list args)
{
    CTASSERT(event != NULL);
    CTASSERT(fmt != NULL);

    char *msg = vformat(fmt, args);

    event->underline = msg;
}

USE_DECL
void msg_note(event_t *event, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    msg_vnote(event, fmt, args);

    va_end(args);
}

USE_DECL
void msg_vnote(event_t *event, const char *fmt, va_list args)
{
    CTASSERT(event != NULL);
    CTASSERT(fmt != NULL);

    char *msg = vformat(fmt, args);

    if (event->notes == NULL)
    {
        event->notes = vector_new(4);
    }

    vector_push(&event->notes, msg);
}
