// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "ctu/sema/attrib.h"

#include "ctu/ast.h"

#include "cthulhu/tree/tree.h"

typedef struct language_runtime_t language_runtime_t;

typedef enum ctu_tag_t
{
#define DECL_TAG(ID, INIT, STR) ID INIT,
#include "ctu/ctu.inc"
    eCtuTagTotal
} ctu_tag_t;

///
/// sema context
///

typedef struct ctu_sema_t {
    tree_t *sema; ///< current scope
    tree_t *decl; ///< current decl
    vector_t *block; ///< current statement block
    tree_t *current_loop; ///< current loop
} ctu_sema_t;

ctu_sema_t ctu_sema_init(tree_t *sema, tree_t *decl, vector_t *block);
ctu_sema_t ctu_sema_nested(ctu_sema_t *parent, tree_t *sema, tree_t *decl, vector_t *block);
logger_t *ctu_sema_reports(ctu_sema_t *sema);

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

tree_t *ctu_current_loop(ctu_sema_t *sema);
void ctu_set_current_loop(ctu_sema_t *sema, tree_t *loop);

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

void ctu_rt_mod(language_runtime_t *runtime, tree_t *root);
