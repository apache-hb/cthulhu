#pragma once

#include "ctu/ast/ast.h"
#include "ctu/ast/scan.h"
#include "ctu/util/util.h"

node_t *pl0_odd(scan_t *scan, where_t where, node_t *expr);
node_t *pl0_print(scan_t *scan, where_t where, node_t *expr);
node_t *pl0_module(scan_t *scan, where_t where, vector_t *consts, vector_t *values, vector_t *procs, node_t *body);
node_t *pl0_procedure(scan_t *scan, where_t where, node_t *name, vector_t *locals, node_t *body);
