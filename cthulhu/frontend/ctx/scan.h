#pragma once

#include "ast.h"
#include "cthulhu/ast/ast.h"
#include "cthulhu/ast/compile.h"

scan_t ctx_open(reports_t *reports, file_t *file);
ctx_t *ctx_compile(scan_t *scan);

#define CTXLTYPE where_t
