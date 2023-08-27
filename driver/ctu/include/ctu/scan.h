#pragma once

#include <gmp.h>

#include "scan/scan.h"

#include "std/vector.h"

typedef struct ctu_digit_t {
    mpz_t value;
} ctu_digit_t;

typedef struct ctu_string_t {
    char *text;
    size_t length;
} ctu_string_t;

ctu_digit_t ctu_parse_digit(scan_t *scan, const char *str, size_t base);

ctu_string_t ctu_parse_string(scan_t *scan, const char *str, size_t length);

#define CTULTYPE where_t
