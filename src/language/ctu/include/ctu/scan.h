// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "core/where.h"

typedef struct scan_t scan_t;
typedef struct vector_t vector_t;

typedef struct ctu_integer_t ctu_integer_t;

#define CTULTYPE where_t

void ctu_parse_digit(scan_t *scan, where_t where, ctu_integer_t *integer, const char *str, int len, size_t base);
