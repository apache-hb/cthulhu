#pragma once

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ast.h"
#include "ctu/lir/lir.h"

c_t *c_parse(reports_t *reports, file_t *file);
lir_t *c_analyze(reports_t *reports, c_t *node);
