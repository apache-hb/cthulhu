#pragma once

#include "cthulhu/hlir/digit.h"
#include "scan/scan.h"

#include <gmp.h>

typedef struct ast_t ast_t;
typedef struct vector_t vector_t;

typedef struct
{
    digit_t digit;
    sign_t sign;
} suffix_t;

void init_scan(scan_t scan);
void enter_template(scan_t scan);
size_t exit_template(scan_t scan);

void add_attribute(scan_t scan, ast_t *ast);
vector_t *collect_attributes(scan_t scan);

void init_string_with_suffix(suffix_t *suffix, mpz_t mpz, char *text, size_t base);

#define CTULTYPE where_t
