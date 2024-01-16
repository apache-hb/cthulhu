#pragma once

#include "scan/node.h"

#include "cthulhu/events/events.h"

typedef struct logger_t logger_t;
typedef struct scan_t scan_t;

typedef struct cpp_scan_t
{
    logger_t *reports;
} cpp_scan_t;

cpp_scan_t *cpp_scan_context(scan_t *scan);
