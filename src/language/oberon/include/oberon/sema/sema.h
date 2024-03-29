// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cthulhu/tree/query.h"

typedef struct language_runtime_t language_runtime_t;

typedef enum obr_tag_t {
#define DECL_TAG(ID, VAL, STR) ID VAL,
#include "oberon/oberon.inc"

    eObrTagTotal
} obr_tag_t;

/// getters

tree_t *obr_get_symbol(tree_t *sema, obr_tag_t tag, const char *name);

tree_t *obr_get_type(tree_t *sema, const char *name);
tree_t *obr_get_module(tree_t *sema, const char *name);
tree_t *obr_get_namespace(tree_t *sema, const char *name);

/// add decls

void obr_add_decl(tree_t *sema, obr_tag_t tag, const char *name, tree_t *decl);

/// builtin types

// basic types as defined in [6.1 Basic Types]

tree_t *obr_get_bool_type(void); // BOOLEAN
tree_t *obr_get_char_type(void); // CHAR
tree_t *obr_get_shortint_type(void); // SHORTINT
tree_t *obr_get_integer_type(void); // INTEGER
tree_t *obr_get_string_type(const node_t *node, size_t length); // STRING
tree_t *obr_get_longint_type(void); // LONGINT
tree_t *obr_get_real_type(void); // REAL
tree_t *obr_get_longreal_type(void); // LONGREAL
tree_t *obr_get_void_type(void); // VOID

/// runtime module

void obr_create(language_runtime_t *runtime, tree_t *root);
