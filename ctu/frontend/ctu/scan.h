#pragma once

#include "ast.h"
#include "ctu/ast/ast.h"
#include "ctu/ast/compile.h"

ctu_t *ctu_compile(reports_t *reports, file_t *fd);

#define CTULTYPE where_t
