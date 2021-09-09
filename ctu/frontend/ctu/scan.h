#pragma once

#include "ctu/ast/ast.h"
#include "ctu/ast/compile.h"

node_t *ctu_compile(reports_t *reports, file_t *fd);

#define CTULTYPE where_t
