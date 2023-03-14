#pragma once

#include "cthulhu/hlir/digit.h"
#include "scan/scan.h"

#include <gmp.h>

typedef struct ast_t ast_t;
typedef struct vector_t vector_t;

typedef struct string_t {
    char *text;
    size_t size;
} string_t;

void init_scan(scan_t *scan);
void enter_template(scan_t *scan);
size_t exit_template(scan_t *scan);

void add_attribute(scan_t *scan, ast_t *ast);
vector_t *collect_attributes(scan_t *scan);

char *init_string_with_suffix(mpz_t mpz, const char *text, int base);

string_t parse_string_escapes(reports_t *reports, const char *text, size_t len);

#define CTULTYPE where_t
