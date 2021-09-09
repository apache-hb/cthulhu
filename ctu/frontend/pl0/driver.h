#pragma once

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ast.h"
#include "ctu/lir/lir.h"

pl0_t *pl0_parse(reports_t *reports, file_t *file);
lir_t *pl0_analyze(reports_t *reports, pl0_t *node);
