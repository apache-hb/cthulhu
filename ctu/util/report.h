#pragma once

#include "ctu/ast/ast.h"

#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum {
    INTERNAL, /* compiler reached a broken state */
    ERROR, /* an issue that prevents compilation from continuing */
    WARNING, /* an issue that should be addressed */
    NOTE /* an info message */
} level_t;

typedef struct {
    char *message;

    const node_t *node;
} part_t;

typedef struct {
    /* the level of this error */
    level_t level;

    /* error message displayed at the top */
    char *message;
    char *underline;

    vector_t *parts;

    /* source and location, if node is NULL then location is ignored */
    const node_t *node;

    /* extra note */
    char *note;
} message_t;

typedef struct {
    vector_t *messages;
} reports_t;

reports_t *begin_reports();
int end_reports(reports_t *reports, size_t limit, const char *name);

message_t *assert2(reports_t *reports, const char *fmt, ...);

message_t *report2(reports_t *reports, level_t level, const node_t *node, const char *fmt, ...);
void report_append2(message_t *message, const node_t *node, const char *fmt, ...);
void report_underline(message_t *message, const char *fmt, ...);
void report_note2(message_t *message, const char *fmt, ...);
