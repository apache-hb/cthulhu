#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/file.h"
#include "cthulhu/util/report.h"

void c89_emit_modules(reports_t *reports, vector_t *modules, file_t output);
void wasm_emit_modules(reports_t *reports, vector_t *modules, file_t output);
