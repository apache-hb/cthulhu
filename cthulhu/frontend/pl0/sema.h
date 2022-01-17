#pragma once

#include "cthulhu/driver/cmd.h"
#include "ast.h"
#include "cthulhu/lir/sema.h"
#include "cthulhu/lir/lir.h"

lir_t *pl0_sema(reports_t *reports, settings_t *settings, pl0_t *node);
