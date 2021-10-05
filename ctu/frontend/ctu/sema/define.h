#pragma once

#include "data.h"

lir_t *compile_define(lir_t *lir);
void build_define(sema_t *sema, lir_t *lir);
