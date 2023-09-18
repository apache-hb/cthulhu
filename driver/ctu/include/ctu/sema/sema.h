#pragma once

#include "ctu/sema/attrib.h"

#include "ctu/ast.h"

#include "cthulhu/tree/tree.h"

typedef struct lifetime_t lifetime_t;

typedef enum ctu_tag_t {
    eCtuTagValues = eSemaValues,
    eCtuTagTypes = eSemaTypes,
    eCtuTagFunctions = eSemaProcs,
    eCtuTagModules = eSemaModules,

    eCtuTagImports,
    eCtuTagAttribs,
    eCtuTagSuffixes,
    eCtuTagLabels,

    eCtuTagTotal
} ctu_tag_t;

///
/// sema context
///

typedef struct ctu_sema_t {
    tree_t *sema; ///< current scope
    tree_t *decl; ///< current decl
    vector_t *block; ///< current statement block
} ctu_sema_t;

ctu_sema_t ctu_sema_init(tree_t *sema, tree_t *decl, vector_t *block);
ctu_sema_t ctu_sema_enter_scope(ctu_sema_t sema, tree_t *scope, vector_t *block);
reports_t *ctu_sema_reports(ctu_sema_t sema);

///
/// getting decls
///

tree_t *ctu_get_namespace(tree_t *sema, const char *name, bool *imported);
tree_t *ctu_get_type(tree_t *sema, const char *name);
tree_t *ctu_get_decl(tree_t *sema, const char *name);
tree_t *ctu_get_loop(tree_t *sema, const char *name);
ctu_attrib_t *ctu_get_attrib(tree_t *sema, const char *name);

///
/// adding decls
///

void ctu_add_decl(tree_t *sema, ctu_tag_t tag, const char *name, tree_t *decl);

///
/// extras
///

tree_t *ctu_current_loop(tree_t *sema);
void ctu_set_current_loop(tree_t *sema, tree_t *loop);

///
/// builtin types
///

tree_t *ctu_get_int_type(digit_t digit, sign_t sign);
tree_t *ctu_get_char_type(void);
tree_t *ctu_get_bool_type(void);
tree_t *ctu_get_void_type(void);

///
/// runtime module
///

vector_t *ctu_rt_path(void);
tree_t *ctu_rt_mod(lifetime_t *lifetime);
