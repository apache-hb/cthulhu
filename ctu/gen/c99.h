#pragma once

#include "ctu/ir/ir.h"
#include <stdio.h>

/**
 * a c99 backend to use until the various required
 * x86/x64 backends work
 */

void gen_c99(FILE *out, module_t *mod);
