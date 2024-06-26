// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/compiler.h"
#include "cthulhu/tree/tree.h"

typedef struct node_t node_t;
typedef struct tree_t tree_t;

CT_BEGIN_API

CT_TREE_API const char *tree_kind_to_string(tree_kind_t kind);
CT_TREE_API const char *tree_to_string(const tree_t *self);
CT_TREE_API char *tree_to_string_arena(const tree_t *self, arena_t *arena);

CT_TREE_API const tree_attribs_t *tree_get_attrib(const tree_t *self);

CT_TREE_API bool tree_has_vis(const tree_t *self, tree_visibility_t visibility);

CT_TREE_API bool tree_has_storage(const tree_t *self);

///
/// storage decl queries
///

CT_TREE_API tree_quals_t tree_get_storage_quals(const tree_t *self);
CT_TREE_API const tree_t *tree_get_storage_type(const tree_t *self);
CT_TREE_API size_t tree_get_storage_size(const tree_t *self);

///
/// enum queries
///

CT_PUREFN
CT_TREE_API vector_t *tree_enum_get_cases(const tree_t *self);

///
/// closure + function queries
///

CT_PUREFN
CT_TREE_API const tree_t *tree_fn_get_return(const tree_t *self);

CT_PUREFN
CT_TREE_API const vector_t *tree_fn_get_params(const tree_t *self);

CT_PUREFN
CT_TREE_API tree_arity_t tree_fn_get_arity(const tree_t *self);

///
/// type queries
///

CT_PUREFN
CT_TREE_API tree_t *tree_ty_get_field(const tree_t *self, const char *name);

CT_PUREFN
CT_TREE_API tree_t *tree_ty_get_case(const tree_t *self, const char *name);

CT_PUREFN
CT_TREE_API bool tree_ty_is_address(const tree_t *type);

CT_PUREFN
CT_TREE_API tree_quals_t tree_ty_get_quals(const tree_t *self);

/**
 * @brief get the type of a type after it has been loaded
 *
 * @note pointer -> type, reference -> type, etc.
 * @param self the type to load from
 * @return const tree_t* the loaded type
 */
CT_PUREFN
CT_TREE_API const tree_t *tree_ty_load_type(const tree_t *self);

CT_END_API
