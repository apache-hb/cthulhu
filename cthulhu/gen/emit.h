#pragma once

#include "cthulhu/util/util.h"
#include "cthulhu/util/report.h"
#include "cthulhu/lir/lir.h"

#include "value.h"
#include "operand.h"
#include "module.h"

module_t *module_build(reports_t *reports, const char *base, vector_t *nodes);
void module_print(FILE *out, module_t *mod);
