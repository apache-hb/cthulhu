#pragma once

#include "scan/node.h" // IWYU pragma: export

#define CCLTYPE where_t

typedef struct c_ast_t c_ast_t;
typedef struct logger_t logger_t;

typedef struct cc_scan_t
{
    logger_t *logger;
} cc_scan_t;

cc_scan_t *cc_scan_context(scan_t *scan);

c_ast_t *cc_get_typedef_name(cc_scan_t *scan, const char *name);

void cc_on_error(scan_t *scan, const char *msg, where_t where);
