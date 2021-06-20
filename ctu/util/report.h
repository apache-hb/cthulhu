#pragma once

#include "ctu/ast/ast.h"

/**
 * fast assert
 * ASSERT(expr)("error message %s", "more");
 */
void assert(const char *fmt, ...);
void ensure(const char *fmt, ...);
#define ENSURE(expr) if (!(expr)) (ensure)
#define ASSERT(expr) if (!(expr)) (assert)

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

/**
 * set the report limit and begin a new report
 */
void report_begin(size_t limit);
/**
 * end the current report of `name`
 * return true if there were fatal errors
 * otherwise return false
 */
bool report_end(const char *name);

/**
 * generate a report from a node
 */
void reportf(level_t level, node_t *node, const char *fmt, ...);

/**
 * generate a report from a source and location
 */
void report(level_t level, scanner_t *source, where_t where, const char *fmt, ...);
