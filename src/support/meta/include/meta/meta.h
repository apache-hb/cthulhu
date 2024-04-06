// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "core/analyze.h"

typedef struct meta_ast_t meta_ast_t;
typedef struct io_t io_t;

void meta_emit_common(IN_NOTNULL io_t *header, IN_NOTNULL io_t *source);

void meta_emit_cmdline(IN_NOTNULL const meta_ast_t *ast, IN_NOTNULL io_t *header, IN_NOTNULL io_t *source);
