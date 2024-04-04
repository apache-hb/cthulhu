// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "scan/node.h" // IWYU pragma: export
#include "core/where.h"

#include <gmp.h>

#ifndef JSONLTYPE
#   define JSONLTYPE json_where_t
#endif

typedef struct logger_t logger_t;
typedef struct set_t set_t;

typedef struct json_where_t
{
    size_t offset;
    where_t where;
} json_where_t;

typedef struct json_scan_t
{
    logger_t *reports;
} json_scan_t;

CT_LOCAL json_scan_t *json_scan_context(scan_t *scan);

CT_LOCAL void json_parse_integer(mpz_t integer, scan_t *scan, where_t where, const char *text, int base);
CT_LOCAL void json_parse_float(float *real, scan_t *scan, where_t where, const char *text);
CT_LOCAL void json_parse_string(text_view_t *string, scan_t *scan, json_where_t where, const char *text, size_t length);
