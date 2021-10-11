#pragma once

#include "data.h"

type_t *compile_type(sema_t *sema, ctu_t *ctu);

void add_struct(sema_t *sema, ctu_t *ctu);
void add_union(sema_t *sema, ctu_t *ctu);
