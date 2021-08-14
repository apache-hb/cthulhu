#pragma once

#include "ctu/ast/scan.h"
#include "ast.h"

scan_t *pl0_scan_file(const char *path, FILE *fd);
pl0_node_t *pl0_compile(const char *path, FILE *fd);

#define PL0LTYPE where_t
