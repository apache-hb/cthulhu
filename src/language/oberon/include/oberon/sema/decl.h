#pragma once

#include "oberon/ast.h"

#include "oberon/sema/sema.h"

typedef struct obr_forward_t {
    obr_tag_t tag;
    tree_t *decl;
} obr_forward_t;

obr_forward_t obr_forward_decl(tree_t *sema, obr_t *decl);

tree_t *obr_add_init(tree_t *sema, obr_t *mod, tree_context_t *tree_context);
