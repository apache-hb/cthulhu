#pragma once

#include <gmp.h>

#include "scan/scan.h"

#include "std/vector.h"

typedef struct ctu_digit_t {
    mpz_t value;
    char *suffix;
} ctu_digit_t;

typedef struct ctu_string_t {
    char *text;
    size_t length;
} ctu_string_t;

ctu_digit_t ctu_parse_digit(const char *str, size_t base);

ctu_string_t ctu_parse_string(reports_t *reports, const char *str, size_t length);

#define CTULTYPE where_t
