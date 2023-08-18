#pragma once

#include "oberon/ast.h"

#include "cthulhu/tree/query.h"

typedef struct lifetime_t lifetime_t;

typedef enum obr_tag_t {
    eObrTagValues = eSema2Values,
    eObrTagTypes = eSema2Types,
    eObrTagProcs = eSema2Procs,
    eObrTagModules = eSema2Modules,

    eObrTagTotal
} obr_tags_t;

tree_t *obr_get_type(tree_t *sema, const char *name);
tree_t *obr_get_module(tree_t *sema, const char *name);

void obr_add_decl(tree_t *sema, obr_tags_t tag, const char *name, tree_t *decl);

tree_t *obr_get_digit_type(digit_t digit, sign_t sign);
tree_t *obr_get_bool_type(void);

tree_t *obr_rt_mod(lifetime_t *lifetime);
vector_t *obr_rt_path(void);
