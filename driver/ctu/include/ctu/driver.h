#pragma once

typedef struct driver_t driver_t;
typedef struct context_t context_t;

void ctu_init(driver_t *handle);

void ctu_forward_decls(context_t *context);
void ctu_process_imports(context_t *context);
void ctu_compile_module(context_t *context);
