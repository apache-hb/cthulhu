#pragma once

#include "ctu/frontend/ctu/sema/data.h"

lir_t *retype_expr(sema_t *sema, const type_t *type, lir_t *lir);
