#pragma once

#include "cthulhu/mediator/mediator.h"

void pl0_init(lang_handle_t *runtime);

hlir_t *pl0_forward_decls(lang_handle_t *runtime, context_t *ctx);
void pl0_process_imports(lang_handle_t *runtime, context_t *ctx);
hlir_t *pl0_compile_module(lang_handle_t *runtime, io_t *io);
