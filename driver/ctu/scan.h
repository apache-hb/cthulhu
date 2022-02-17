#pragma once

#include "cthulhu/ast/compile.h"

#include <gmp.h>

#define CTULTYPE where_t

typedef struct {
    size_t depth; // template depth
} lex_extra_t;

void enter_template(scan_t *scan);
size_t exit_template(scan_t *scan);
