#pragma once

#include "ast.h"
#include "cthulhu/util/report.h"
#include "cthulhu/hlir/hlir.h"

void pl0_init();
hlir_t *pl0_sema(reports_t *reports, void *node);
