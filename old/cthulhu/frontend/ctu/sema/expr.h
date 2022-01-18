#pragma once

#include "data.h"

lir_t *compile_expr(sema_t *sema, ctu_t *expr);

lir_t *compile_stmts(sema_t *sema, ctu_t *stmts);
lir_t *compile_stmt(sema_t *sema, ctu_t *stmt);
