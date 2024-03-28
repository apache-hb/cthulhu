// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "scan/node.h" // IWYU pragma: export

#define CCLTYPE where_t

typedef struct c_ast_t c_ast_t;
typedef struct logger_t logger_t;

c_ast_t *cc_get_typedef_name(scan_t *scan, const char *name);

void cc_on_error(scan_t *scan, const char *msg, where_t where);
