#include "notify/notify.h"

#include "base/panic.h"
#include "memory/arena.h"

#include "memory/memory.h"
#include "std/map.h"
#include "std/set.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

typedef struct logger_t
{
    arena_t *arena;

    typevec_t *messages;
} logger_t;

USE_DECL
logger_t *logger_new(arena_t *arena)
{
    CTASSERT(arena != NULL);

    logger_t *logs = ARENA_MALLOC(arena, sizeof(logger_t), "logger", NULL);

    logs->arena = arena;
    logs->messages = typevec_new(sizeof(event_t), 8, arena);

    ARENA_IDENTIFY(arena, logs->messages, "messages", logs);

    return logs;
}

USE_DECL
typevec_t *logger_get_events(const logger_t *logs)
{
    CTASSERT(logs != NULL);

    return logs->messages;
}

USE_DECL
bool logger_has_errors(const logger_t *logs, notify_rules_t rules)
{
    CTASSERT(logs != NULL);
    CTASSERT(rules.ignored_warnings != NULL);
    CTASSERT(rules.warnings_as_errors != NULL);

    const typevec_t *events = logger_get_events(logs);
    size_t len = typevec_len(events);

    for (size_t i = 0; i < len; i++)
    {
        event_t event;
        typevec_get(events, i, &event);

        const diagnostic_t *diagnostic = event.diagnostic;
        CTASSERTF(diagnostic != NULL, "event %zu has no diagnostic", i);

        switch (diagnostic->severity)
        {
        case eSeverityFatal:
        case eSeverityInternal:
        case eSeveritySorry:
            return true;

        case eSeverityWarn:
            if (set_contains_ptr(rules.ignored_warnings, diagnostic))
            {
                continue;
            }

            if (set_contains_ptr(rules.warnings_as_errors, diagnostic))
            {
                return true;
            }
            break;

        default:
            break;
        }
    }

    return false;
}

USE_DECL
void logger_reset(logger_t *logs)
{
    CTASSERT(logs != NULL);

    typevec_reset(logs->messages);
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

    char *msg = vformat(fmt, args);

    event_t event = {
        .diagnostic = diagnostic,
        .node = node,
        .message = msg,
        .segments = NULL,
        .notes = NULL,
    };

    return typevec_push(logs->messages, &event);
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
        arena_t *arena = get_global_arena();
        event->segments = typevec_new(sizeof(segment_t), 2, arena);
        ARENA_IDENTIFY(arena, event->segments, "segments", event);
    }

    char *msg = vformat(fmt, args);

    segment_t segment = {
        .node = node,
        .message = msg
    };

    typevec_push(event->segments, &segment);
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
