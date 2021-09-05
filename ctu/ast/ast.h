#pragma once

#include "scan.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

typedef enum {
    BINARY_ADD,
    BINARY_SUB,
    BINARY_MUL,
    BINARY_DIV,
    BINARY_REM,

    BINARY_EQ,
    BINARY_NEQ,
    BINARY_GT,
    BINARY_GTE,
    BINARY_LT,
    BINARY_LTE,
} binary_t;

typedef enum {
    UNARY_NEG,
    UNARY_ABS,
} unary_t;

typedef struct {
    scan_t *scan;
    where_t where;
} node_t;

const char *binary_name(binary_t op);
const char *unary_name(unary_t op);

node_t *node_new(scan_t *scan, where_t where);
report_t reportn(level_t lvl, const node_t *node, const char *fmt, ...);
void report_appendn(report_t id, const node_t *node, const char *fmt, ...);
