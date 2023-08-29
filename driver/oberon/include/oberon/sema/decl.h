#pragma once

#include "oberon/sema/sema.h"

typedef struct obr_forward_t {
    obr_tag_t tag;
    tree_t *decl;
} obr_forward_t;

obr_forward_t obr_forward_decl(tree_t *sema, obr_t *decl);
