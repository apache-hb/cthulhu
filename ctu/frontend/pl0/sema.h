#pragma once

#include "ast.h"
#include "ctu/lir/sema.h"
#include "ctu/lir/lir.h"

lir_t *pl0_sema(reports_t *reports, pl0_t *node);
