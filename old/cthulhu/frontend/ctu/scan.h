#pragma once

#include "ast.h"
#include "cthulhu/ast/ast.h"
#include "cthulhu/ast/compile.h"

scan_t ctu_open(reports_t *reports, file_t *file);
ctu_t *ctu_compile(scan_t *scan);

#define CTULTYPE where_t
