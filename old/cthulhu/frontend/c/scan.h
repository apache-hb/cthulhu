#pragma once

#include "ast.h"
#include "sema.h"
#include "cthulhu/ast/scan.h"
#include "cthulhu/util/report.h"

c_t *c_compile(reports_t *reports, file_t *fd);
