#pragma once

#include "ctu/ast/ast.h"

void assert(const char *fmt, ...);
#define ASSERT(expr) if (!(expr)) (assert)

typedef enum {
    LEVEL_ERROR,
    LEVEL_WARNING
} level_t;

void report_begin(size_t limit);
bool report_end(const char *name);
void reportf(level_t level, node_t *node, const char *fmt, ...);
void report(level_t level, scanner_t *source, where_t where, const char *fmt, ...);
