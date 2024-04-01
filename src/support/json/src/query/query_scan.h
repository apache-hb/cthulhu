// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "scan/node.h" // IWYU pragma: export

#include <gmp.h>

#define QUERYLTYPE where_t

typedef struct logger_t logger_t;

typedef struct query_scan_t
{
    logger_t *reports;
} query_scan_t;

query_scan_t *query_scan_context(scan_t *scan);

void query_parse_integer(mpz_t integer, scan_t *scan, where_t where, const char *text, int base);
void query_parse_string(text_t *string, scan_t *scan, where_t where, const char *text, size_t length);
