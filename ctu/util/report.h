#pragma once

#include <stddef.h>
#include <stdarg.h>

#include "ctu/ast/ast.h"
#include "util.h"

typedef enum {
    INTERNAL, /* compiler reached a broken state */
    ERROR, /* an issue that prevents compilation from continuing */
    WARNING, /* an issue that should be addressed */
    NOTE /* an info message */
} level_t;

typedef struct reports_t {
    level_t level;

    char *message;
    char *underline;

    vector_t *parts;

    const node_t *node;

    char *note;
} message_t;

typedef struct {
    size_t limit;
    size_t used;

    message_t messages[]; 
} reports_t;

reports_t *begin_report(size_t limit);
int end_report(reports_t *reports, const char *name);

message_t *assertf(reports_t *reports, const char *fmt, ...);
#define ASSERTF(ctx, expr, ...) ((!(expr)) ? assertf(reports, __VA_ARGS__)) : NULL

message_t *reportf(reports_t *reports, level_t level, const node_t *node, const char *fmt, ...);
message_t *reportv(reports_t *reports, level_t level, const node_t *node, const char *fmt, va_list args);

void report_appendf(message_t *message, const node_t *node, const char *fmt, ...);
void report_appendv(message_t *message, const node_t *node, const char *fmt, va_list args);
void report_note(message_t *message, const char *fmt, ...);

#if 0
#pragma once

#include "ctu/ast/scan.h"

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
#endif
