#pragma once

#include "ctu/util/util.h"
#include "ast.h"
#include "ctu/lir/lir.h"

pl0_t *pl0_parse(file_t *file);
lir_t *pl0_analyze(pl0_t *node);
