#pragma once

#include <gmp.h>

#include "scan/node.h"
#include "scan/scan.h"

#include "std/vector.h"

typedef struct logger_t logger_t;

typedef struct ctu_scan_t
{
    logger_t *reports;
    vector_t *attribs;
} ctu_scan_t;

typedef struct ctu_digit_t
{
    mpz_t value;
} ctu_digit_t;

ctu_scan_t *ctu_scan_context(scan_t *scan);

ctu_digit_t ctu_parse_digit(scan_t *scan, where_t where, const char *str, size_t base);

#define CTULTYPE where_t