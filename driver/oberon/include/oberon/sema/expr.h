#pragma once

#include "oberon/sema/sema.h"

h2_t *obr_sema_rvalue(h2_t *sema, obr_t *expr, h2_t *implicitType);
h2_t *obr_default_value(const node_t *node, const h2_t *type);
