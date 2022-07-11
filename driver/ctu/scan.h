#pragma once

#include "scan/scan.h"

typedef struct ast_t ast_t;
typedef struct vector_t vector_t;

void init_scan(scan_t scan);
void enter_template(scan_t scan);
size_t exit_template(scan_t scan);

void add_attribute(scan_t scan, ast_t *ast);
vector_t *collect_attributes(scan_t scan);

#define CTULTYPE where_t
