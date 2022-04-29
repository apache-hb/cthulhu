#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report.h"

void c89_emit_tree(reports_t *reports, const hlir_t *hlir);
void wasm_emit_tree(reports_t *reports, const hlir_t *hlir);

void c89_emit_modules(reports_t *reports, vector_t *modules);
