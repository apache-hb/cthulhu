#pragma once

#include "cthulhu/ast/scan.h"

void init_scan(scan_t *scan);
void enter_template(scan_t *scan);
size_t exit_template(scan_t *scan);

#define CTULTYPE where_t
