#pragma once

#include "data.h"

void init_attribs(void);
void compile_attribs(sema_t *sema, lir_t *lir, ctu_t *ctu);

lir_t *compile_detail(sema_t *sema, ctu_t *ctu, type_t *type, const char *detail);
