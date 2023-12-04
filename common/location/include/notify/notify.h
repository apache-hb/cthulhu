#pragma once

#include "core/macros.h"

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
    unsigned code;

    const char *brief;
    const char *description;
} diagnostic_t;

reports_t *msg_new(void);
void msg_delete(reports_t *reports);

message_t *msg_diagnostic(reports_t *reports, const diagnostic_t *diagnostic);

message_t *msg_notify(reports_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...);
message_t *msg_vnotify(reports_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, va_list args);

void msg_append(message_t *message, const node_t *node, const char *fmt, ...);

END_API
