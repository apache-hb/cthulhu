#pragma once

#include "ast.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report.h"

void pl0_init();
hlir_t *pl0_sema(reports_t *reports, void *node);
