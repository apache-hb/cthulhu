#pragma once

#include "ast.h"
#include "ctu/lir/sema.h"

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu);
