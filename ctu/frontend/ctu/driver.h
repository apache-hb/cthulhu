#pragma once

#include "ast.h"
#include "ctu/util/util.h"
#include "ctu/ast/ast.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"

ctu_t *ctu_parse(reports_t *reports, file_t *file);
lir_t *ctu_analyze(reports_t *reports, ctu_t *node);
