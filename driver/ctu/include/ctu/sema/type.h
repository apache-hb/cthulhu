#pragma once

#include "ctu/sema/sema.h"

tree_t *ctu_sema_type(tree_t *sema, const ctu_t *type);

bool ctu_type_is(const tree_t *type, tree_kind_t kind);

const char *ctu_type_string(const tree_t *type);
