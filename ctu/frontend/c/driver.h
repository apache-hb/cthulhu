#pragma once

#include "ctu/util/util.h"
#include "ctu/ast/ast.h"
#include "ctu/lir/lir.h"

node_t *c_parse(file_t *file);
lir_t *c_analyze(node_t *node);
