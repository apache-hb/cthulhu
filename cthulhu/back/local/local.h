#pragma once

#include <stdio.h>

#include "cthulhu/middle/ir.h"

typedef struct {
    void *out;
} ctu_context_t;

ctu_context_t *ctu_compile(unit_t *unit);
void ctu_output(ctu_context_t *ctx, FILE *file);
