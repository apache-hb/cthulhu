#pragma once

typedef struct reports_t reports_t;
typedef struct vector_t vector_t;
typedef struct io_t io_t;

typedef struct ssa_module_t ssa_module_t;

/* TODO: add cxx headers and wrappers */
typedef struct {
    reports_t *reports;

    io_t *source;
    io_t *header;
} emit_config_t;

void c89_emit_hlir_modules(emit_config_t config, vector_t *modules);
void emit_ssa_modules(emit_config_t config, ssa_module_t *module);
