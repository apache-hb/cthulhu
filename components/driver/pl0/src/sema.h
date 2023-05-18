#pragma once

#include "cthulhu/mediator/mediator.h"

void pl0_init(lang_handle_t *runtime);

void pl0_forward_decls(lang_handle_t *runtime, void *ast);
void pl0_process_imports(lang_handle_t *runtime);
hlir_t *pl0_compile_module(lang_handle_t *runtime);
