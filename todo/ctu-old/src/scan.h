#pragma once

#include "cthulhu/hlir/h2.h"
#include "scan/scan.h"

#include <gmp.h>

typedef struct ast_t ast_t;
typedef struct vector_t vector_t;
typedef struct driver_t driver_t;

typedef struct string_t {
    char *text;
    size_t size;
} string_t;

void ctu_init_scan(scan_t *scan, driver_t *handle);
void enter_template(scan_t *scan);
size_t exit_template(scan_t *scan);

driver_t *get_lang_handle(scan_t *scan);

void add_attribute(scan_t *scan, ast_t *ast);
vector_t *collect_attributes(scan_t *scan);

char *init_string_with_suffix(mpz_t mpz, const char *text, int base);

string_t parse_string_escapes(reports_t *reports, const char *text, size_t len);

void init_decimal(mpq_t result, const char *text);

#define CTULTYPE where_t
