#pragma once

#include <stdio.h>

#include "cthulhu/middle/ir.h"
#include "cthulhu/util.h"

typedef void llvm_context;

CTU_API llvm_context *llvm_compile(unit_t *unit);
CTU_API void llvm_output(llvm_context *ctx, FILE *file);