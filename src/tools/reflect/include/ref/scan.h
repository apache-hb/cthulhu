#pragma once

#include <gmp.h>

#include "scan/node.h" // IWYU pragma: export

#define REFLTYPE where_t

BEGIN_API

typedef struct logger_t logger_t;
typedef struct typevec_t typevec_t;

typedef struct ref_scan_t
{
    logger_t *reports;
} ref_scan_t;

ref_scan_t *ref_scan_context(scan_t *scan);

void ref_parse_digit(mpz_t digit, scan_t *scan, where_t where, const char *str, size_t base);

typevec_t *stringlist_begin(scan_t *scan, where_t where, text_t text);

typevec_t *stringlist_append(typevec_t *list, text_t text);

END_API
