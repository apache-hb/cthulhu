#pragma once

#include "ctu/ir/ir.h"
#include <stdio.h>

/**
 * an 8086/x86/x66 backend
 */

void gen_x86(FILE *out, module_t *mod);
