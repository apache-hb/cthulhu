#pragma once

#include "ast.h"
#include "ctu/ast/compile.h"

pl0_t *pl0_compile(scan_t *scan);
char *pl0_normalize(const char *ident);

#define PL0LTYPE where_t
