#pragma once

typedef struct reports_t reports_t;
typedef struct vector_t vector_t;
typedef struct io_t io_t;

void c89_emit_hlir_modules(reports_t *reports, vector_t *modules, io_t *dst);
