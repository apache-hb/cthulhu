#pragma once

#include "ctu/ast/ast.h"

#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

typedef bool verbose_t;
typedef size_t report_t;

typedef enum {
    INTERNAL, /* compiler reached a broken state */
    ERROR, /* an issue that prevents compilation from continuing */
    WARNING, /* an issue that should be addressed */
    NOTE /* an info message */
} level_t;

#define INVALID_REPORT SIZE_MAX

/* begin a report that holds up to `limit` reports */
void begin_report(size_t limit);

/* end a report with a `name` and exit if it contains an error */
void end_report(bool quit, const char *name);

/* report an internal compiler error */
void assert(const char *fmt, ...);
#define ASSERT(expr) if (!(expr)) (assert)

report_t report(level_t level, const char *fmt, ...);
report_t reportf(level_t level, const scan_t *scan, where_t where, const char *fmt, ...);
report_t reportv(level_t level, const scan_t *scan, where_t where, const char *fmt, va_list args);

void report_append(report_t id, const scan_t *scan, where_t where, const char *fmt, ...);
void report_appendv(report_t id, const scan_t *scan, where_t where, const char *fmt, va_list args);
void report_note(report_t id, const char *fmt, ...);

typedef struct {
    char *message;

    const scan_t *scan;
    where_t where;

    const node_t *node;
} part_t;

typedef struct {
    /* the level of this error */
    level_t level;

    /* error message displayed at the top */
    char *message;
    char *underline;

    vector_t *parts;

    /* source and location, if scan is NULL then location is ignored */
    const scan_t *scan;
    where_t where;

    const node_t *node;

    /* extra note */
    char *note;
} message_t;

typedef struct {
    vector_t *messages;
} reports_t;

reports_t *begin_reports();
int end_reports(reports_t *reports, size_t limit, const char *name);

message_t *report2(reports_t *reports, level_t level, const node_t *node, const char *fmt, ...);
void report_append2(message_t *message, const node_t *node, const char *fmt, ...);
void report_underline(message_t *message, const char *fmt, ...);
void report_note2(message_t *message, const char *fmt, ...);
