#pragma once

#include "ctu/driver/cmd.h"
#include "ast.h"
#include "ctu/lir/sema.h"
#include "ctu/lir/lir.h"

lir_t *pl0_sema(reports_t *reports, settings_t *settings, pl0_t *node);
