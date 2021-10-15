#pragma once

#include "ast.h"
#include "ctu/ast/ast.h"
#include "ctu/ast/compile.h"

ctu_t *ctu_compile(scan_t *scan);

#define CTULTYPE where_t
