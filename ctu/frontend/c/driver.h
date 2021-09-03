#pragma once

#include "ctu/util/util.h"
#include "ast.h"
#include "ctu/lir/lir.h"

c_t *c_parse(file_t *file);
lir_t *c_analyze(c_t *node);
