#pragma once

#include "ast.h"
#include "ctu/ast/compile.h"

pl0_t *pl0_compile(reports_t *reports, file_t *fd);

#define PL0LTYPE where_t
