#pragma once

#include "cthulhu/mediator/driver.h"

void pl0_init(lifetime_t *lifetime);

void pl0_forward_decls(lifetime_t *lifetime, const char *name, void *ast);
void pl0_process_imports(lang_handle_t *runtime, compile_t *compile);
hlir_t *pl0_compile_module(lang_handle_t *runtime, compile_t *compile);
