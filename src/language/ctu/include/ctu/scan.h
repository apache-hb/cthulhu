#pragma once

#include "core/where.h"

#include <gmp.h>

typedef struct scan_t scan_t;
typedef struct vector_t vector_t;

#define CTULTYPE where_t

void ctu_parse_digit(scan_t *scan, where_t where, mpz_t digit, const char *str, size_t base);
