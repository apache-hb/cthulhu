#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/file.h"
#include "cthulhu/util/report.h"

typedef struct
{
    const char *defaultModule;
} wasm_settings_t;

void c89_emit_modules(reports_t *reports, vector_t *modules, file_t output);
void wasm_emit_modules(reports_t *reports, vector_t *modules, file_t output, wasm_settings_t settings);
