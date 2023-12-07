#include "notify/notify.h"

#include "base/panic.h"
#include "memory/memory.h"
#include "std/vector.h"

#include "core/macros.h"

typedef struct logger_t
{
    vector_t *messages;
} logger_t;

USE_DECL
logger_t *log_new(void)
{
    logger_t *logs = ctu_malloc(sizeof(logger_t));
    logs->messages = vector_new(4);
    return logs;
}

USE_DECL
void log_delete(logger_t *logs)
{
    CTASSERT(logs != NULL);

    ctu_free(logs);
}

USE_DECL
vector_t *log_reset(logger_t *logs)
{
    CTASSERT(logs != NULL);

    vector_t *messages = logs->messages;
    logs->messages = vector_new(4);
    return messages;
}

void msg_diagnostic(logger_t *logs, const diagnostic_t *diagnostic)
{
    CTASSERT(logs != NULL);
    CTASSERT(diagnostic != NULL);
}

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
