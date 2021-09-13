#pragma once

#include "ast.h"
#include "sema.h"
#include "ctu/ast/scan.h"
#include "ctu/util/report.h"

c_t *c_compile(reports_t *reports, file_t *fd);
