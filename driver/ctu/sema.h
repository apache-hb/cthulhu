#pragma once

#include "cthulhu/interface/runtime.h"
#include "cthulhu/hlir/sema.h"

void ctu_forward_decls(runtime_t *runtime, compile_t *compile);
void ctu_process_imports(runtime_t *runtime, compile_t *compile);
void ctu_compile_module(runtime_t *runtime, compile_t *compile);
