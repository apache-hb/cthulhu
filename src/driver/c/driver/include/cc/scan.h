#pragma once

#include "scan/node.h" // IWYU pragma: export

#define CCLTYPE where_t

typedef struct logger_t logger_t;

typedef struct cc_scan_t
{
    logger_t *reports;
} cc_scan_t;

cc_scan_t *cc_scan_context(scan_t *scan);
