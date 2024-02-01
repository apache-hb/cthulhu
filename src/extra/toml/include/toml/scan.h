#pragma once

#include "scan/node.h" // IWYU pragma: export

#define TOMLLTYPE where_t

typedef struct logger_t logger_t;

typedef struct toml_scan_t
{
    logger_t *reports;
} toml_scan_t;

toml_scan_t *toml_scan_context(scan_t *scan);
