#pragma once

#include "data.h"

lir_t *compile_value(lir_t *lir);
void build_value(sema_t *sema, lir_t *lir);
