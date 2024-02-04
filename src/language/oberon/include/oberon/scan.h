#pragma once

#include "scan/node.h" // IWYU pragma: keep

typedef struct logger_t logger_t;

#define OBRLTYPE where_t

typedef struct obr_scan_t
{
    logger_t *logger;
} obr_scan_t;

obr_scan_t *obr_scan_context(scan_t *scan);
