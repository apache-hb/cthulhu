#pragma once

#include <stdio.h>

#include "cthulhu/middle/ir.h"

typedef struct {
    FILE *file;
} c_ctx_t;

c_ctx_t c_compile(unit_t *unit, FILE *file);
void c_output(c_ctx_t *ctx);
