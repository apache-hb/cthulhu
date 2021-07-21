#pragma once

#include "ctu/ast/ast.h"

/**
 * error severity
 * LEVEL_INTERNAL is an internal compiler issue
 * LEVEL_ERROR is fatal
 * LEVEL_WARNING is for diagnostics
 */
typedef enum {
    LEVEL_INTERNAL,
    LEVEL_ERROR,
    LEVEL_WARNING
} level_t;

#define INVALID_REPORT SIZE_MAX

typedef size_t reportid_t;

/**
 * assert is for internal compiler errors
 * ensure is for normal compiler errors
 * ASSERT(expr)("error message %s", "more");
 * ENSURE(expr)("error message %s", "more");
 */
void assert(const char *fmt, ...);
void ensure(const char *fmt, ...);
#define ENSURE(expr) if (!(expr)) (ensure)
#define ASSERT(expr) if (!(expr)) (assert)

reportid_t warnf(const char *fmt, ...);

/**
 * set the report limit and begin a new report
 */
void report_begin(size_t limit, bool eager);

/**
 * end the current report of `name`
 * return true if there were fatal errors
 * otherwise return false
 */
bool report_end(const char *name);

/**
 * generate a report from a node
 */
reportid_t reportf(level_t level, node_t *node, const char *fmt, ...);

/**
 * generate a report from a source and location
 */
void report(level_t level, scanner_t *source, where_t where, const char *fmt, ...);

/**
 * add a message to be printed next to the squiggly report undline
 */
void report_underline(reportid_t id, const char *msg);

/**
 * add a note to be printed under a report
 */
void report_note(reportid_t id, const char *note);

/**
 * set the unique tag for a report
 */
void report_tag(reportid_t id, const void *tag);

extern bool verbose;
extern const where_t NOWHERE;

/**
 * log a message 
 * 
 * i would have liked to call this logf or log
 * but <math.h> had other ideas
 */
void logfmt(const char *fmt, ...);
