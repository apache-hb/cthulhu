#pragma once

#include <stdbool.h>

typedef struct node_t node_t;

typedef enum report_level_t {
    eReportDebug,
    eReportInfo,
    eReportWarn,
    eReportError,

    eReportSorry,
    eReportPanic,

    eReportTotal
} report_level_t;

typedef struct report_t {
    const char *name;
    const char *message;
    report_level_t level;
} report_t;

typedef struct status_t {
    report_level_t level;
    int code;
} status_t;

typedef struct message_t message_t;

typedef struct sink_t sink_t;
typedef struct reports_t reports_t;

bool status_can_continue(status_t status);
int status_get_code(status_t status);

reports_t *reports_create(void);
status_t reports_check(reports_t *self);

message_t *reports_push(reports_t *self, const report_t *id, const char *fmt, ...);

void message_append(message_t *message, const node_t *node, const char *fmt, ...);
void message_underline(message_t *message, const char *fmt, ...);
void message_note(message_t *message, const char *fmt, ...);
