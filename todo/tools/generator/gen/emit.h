#pragma once

#include "ast.h"
#include "cthulhu/util/stream.h"
#include "cthulhu/report/report.h"
#include "std/map.h"

map_t *emit_compiler(reports_t *reports, ast_t *ast);

map_t *emit_tooling(reports_t *reports, ast_t *ast);
