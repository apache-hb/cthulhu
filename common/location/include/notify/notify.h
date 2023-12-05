#pragma once

#include "core/compiler.h"

#include "memory/arena.h"

#include <stdarg.h>

BEGIN_API

typedef struct node_t node_t;

typedef struct reports_t reports_t;
typedef struct sink_t sink_t;
typedef struct message_t message_t;

typedef enum severity_t
{
#include "notify/notify.inc"

    eSeverityTotal
} severity_t;

typedef struct diagnostic_t
{
    severity_t severity;
    const char *id;

    const char *brief;
    const char *description;
} diagnostic_t;

reports_t *msg_new(alloc_t *alloc);
void msg_delete(reports_t *reports);

int msg_flush(reports_t *reports);

message_t *msg_diagnostic(reports_t *reports, const diagnostic_t *diagnostic);

message_t *msg_notify(reports_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...);
message_t *msg_vnotify(reports_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, va_list args);

void msg_append(message_t *message, const node_t *node, const char *fmt, ...);
void msg_underline(message_t *message, const node_t *node, const char *fmt, ...);
void msg_note(message_t *message, const char *fmt, ...);

END_API
