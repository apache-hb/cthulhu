#pragma once

#include "data.h"

lir_t *compile_define(lir_t *lir);
void build_define(sema_t *sema, lir_t *lir);
void add_locals(sema_t *sema, const type_t *type, vector_t *params);
const type_t *lambda_type(sema_t *sema, ctu_t *ctu);
