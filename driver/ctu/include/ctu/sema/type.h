#pragma once

#include "ctu/sema/sema.h"

h2_t *ctu_sema_type(h2_t *sema, const ctu_t *type);

bool ctu_type_is(const h2_t *type, h2_kind_t kind);

const char *ctu_type_string(const h2_t *type);
