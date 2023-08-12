#pragma once

#include "cthulhu/hlir/h2.h"

h2_t *h2_new(h2_kind_t kind, const node_t *node, const h2_t *type);
h2_t *h2_decl(h2_kind_t kind, const node_t *node, const h2_t *type, const char *name);
