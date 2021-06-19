#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "cthulhu/middle/ir.h"

bool gcc_enabled(void);

typedef void gcc_context;

gcc_context *gcc_compile(unit_t *unit, bool debug);
void gcc_output(gcc_context *ctx, const char *file);
