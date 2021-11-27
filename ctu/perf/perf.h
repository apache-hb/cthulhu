#pragma once

#include "ctu/gen/emit.h"

bool dead_function_elimination(reports_t *reports, module_t *mod);

bool dead_step_elimination(reports_t *reports, module_t *mod);

bool dead_label_elimination(reports_t *reports, module_t *mod);

void run_passes(reports_t *reports, module_t *ctx);
