#pragma once

#include "generic.h"
#include "ctu/ir/ir.h"

/**
 * codegen for 8086, x86, and x64
 */

blob_t *gen_x64(module_t *mod);
