#pragma once

#include "cthulhu/ast/compile.h"

#include <gmp.h>

#define CTULTYPE where_t

void init_scan(scan_t *scan);
void enter_template(scan_t *scan);
size_t exit_template(scan_t *scan);