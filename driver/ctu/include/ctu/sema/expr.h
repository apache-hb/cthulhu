#pragma once

#include "ctu/sema/sema.h"

h2_t *ctu_sema_rvalue(h2_t *sema, const ctu_t *expr, h2_t *implicitType);
h2_t *ctu_sema_lvalue(h2_t *sema, const ctu_t *expr, h2_t *implicitType);

h2_t *ctu_sema_stmt(h2_t *sema, h2_t *decl, const ctu_t *stmt);
