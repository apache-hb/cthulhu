#pragma once

#include "ctu/util/util.h"
#include "ctu/ast/ast.h"
#include "ctu/lir/lir.h"

node_t *pl0_parse(file_t *file);
lir_t *pl0_analyze(node_t *node);
