#pragma once

#include "ctu/gen/emit.h"

void dead_function_elimination(reports_t *reports, module_t *mod);

void dead_step_elimination(reports_t *reports, module_t *mod);
