#pragma once

#include "ast.h"
#include "ctu/ast/ast.h"
#include "ctu/ast/compile.h"

scan_t ctu_open(reports_t *reports, file_t *file);
ctu_t *ctu_compile(scan_t *scan);

#define CTULTYPE where_t
