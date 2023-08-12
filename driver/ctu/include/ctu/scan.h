#pragma once

#include <gmp.h>

#include "scan/scan.h"

#include "std/vector.h"

typedef struct ctu_digit_t {
    mpz_t value;
    char *suffix;
} ctu_digit_t;

ctu_digit_t ctu_parse_digit(const char *str, size_t base);

#define CTULTYPE where_t
