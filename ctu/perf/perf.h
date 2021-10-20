#pragma once

#include "ctu/gen/emit.h"

void dead_function_elimination(reports_t *reports, module_t *mod);

void dead_step_elimination(reports_t *reports, module_t *mod);

bool pass_dfe(reports_t *reports, module_t *mod);
bool pass_dse(reports_t *reports, module_t *mod);
bool pass_folding(reports_t *reports, module_t *mod);
bool pass_mem2reg(reports_t *reports, module_t *mod);

map_t *get_passes(void);
void apply_passes(reports_t *reports, vector_t *passes, module_t *mod);
