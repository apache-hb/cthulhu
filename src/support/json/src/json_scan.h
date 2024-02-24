// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "scan/node.h" // IWYU pragma: export

#include <gmp.h>

#define JSONLTYPE where_t

typedef struct logger_t logger_t;

typedef struct json_scan_t
{
    logger_t *reports;
} json_scan_t;

json_scan_t *json_scan_context(scan_t *scan);

void json_parse_integer(mpz_t integer, scan_t *scan, where_t where, const char *text, int base);
void json_parse_float(float *real, scan_t *scan, where_t where, const char *text);
void json_parse_string(text_t *string, scan_t *scan, where_t where, const char *text, size_t length);
