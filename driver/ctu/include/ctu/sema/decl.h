#pragma once

#include "ctu/sema/sema.h"

typedef struct ctu_forward_t {
    ctu_tag_t tag;
    h2_t *decl;
} ctu_forward_t;

ctu_forward_t ctu_forward_decl(h2_t *sema, ctu_t *decl);

h2_t *ctu_forward_global(h2_t *sema, ctu_t *decl);
h2_t *ctu_forward_function(h2_t *sema, ctu_t *decl);
