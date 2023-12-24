#pragma once

#include "ctu/sema/sema.h"

typedef struct ctu_forward_t {
    ctu_tag_t tag;
    tree_t *decl;
} ctu_forward_t;

ctu_forward_t ctu_forward_decl(tree_t *sema, ctu_t *decl);
