#pragma once

#include "ctu/ast/scan.h"

#include <stdbool.h>
#include <stddef.h>

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

void report_append(report_t id, const scan_t *scan, where_t where, const char *fmt, ...);
void report_note(report_t id, const char *fmt, ...);
