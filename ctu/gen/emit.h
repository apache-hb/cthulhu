#pragma once

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"

#include "value.h"
#include "operand.h"
#include "module.h"

module_t *module_build(reports_t *reports, lir_t *root);
void module_print(FILE *out, module_t *mod);
