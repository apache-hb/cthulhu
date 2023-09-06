#pragma once

#include "ctu/ast.h"

#include "cthulhu/tree/tree.h"

typedef struct lifetime_t lifetime_t;

typedef enum ctu_tag_t {
    eCtuTagValues = eSema2Values,
    eCtuTagTypes = eSema2Types,
    eCtuTagFunctions = eSema2Procs,
    eCtuTagModules = eSema2Modules,

    eCtuTagImports,
    eCtuTagAttribs,
    eCtuTagSuffixes,

    eCtuTagTotal
} ctu_tag_t;

///
/// getting decls
///

tree_t *ctu_get_namespace(tree_t *sema, const char *name);
tree_t *ctu_get_type(tree_t *sema, const char *name);
tree_t *ctu_get_decl(tree_t *sema, const char *name);

///
/// adding decls
///

void ctu_add_decl(tree_t *sema, ctu_tag_t tag, const char *name, tree_t *decl);

///
/// builtin types
///

tree_t *ctu_get_int_type(digit_t digit, sign_t sign);
tree_t *ctu_get_char_type(void);
tree_t *ctu_get_bool_type(void);
tree_t *ctu_get_void_type(void);
tree_t *ctu_get_str_type(size_t length);

///
/// runtime module
///

vector_t *ctu_rt_path(void);
tree_t *ctu_rt_mod(lifetime_t *lifetime);
