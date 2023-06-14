#pragma once

#include "cthulhu/mediator/driver.h"

void pl0_init(driver_t *handle, ap_t *ap);

void pl0_forward_decls(context_t *context);
void pl0_process_imports(context_t *context);
void pl0_compile_module(context_t *context);
