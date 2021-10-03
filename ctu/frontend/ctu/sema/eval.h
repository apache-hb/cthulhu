#pragma once

#include "data.h"

lir_t *fold_expr(sema_t *sema, lir_t *expr);
lir_t *type_expr(sema_t *sema, lir_t *expr);
