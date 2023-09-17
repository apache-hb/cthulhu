#pragma once

#include "cthulhu/tree/tree.h"

typedef struct reports_t reports_t;
typedef struct node_t node_t;
typedef struct tree_t tree_t;

const char *tree_kind_to_string(tree_kind_t kind);
const char *tree_to_string(const tree_t *self);

const node_t *tree_get_node(const tree_t *self);
const char *tree_get_name(const tree_t *self);
tree_kind_t tree_get_kind(const tree_t *self);
const tree_t *tree_get_type(const tree_t *self);
const tree_attribs_t *tree_get_attrib(const tree_t *self);

bool tree_is(const tree_t *self, tree_kind_t kind);

bool tree_has_vis(const tree_t *self, visibility_t visibility);

///
/// storage decl queries
///

quals_t tree_get_storage_quals(const tree_t *self);
const tree_t *tree_get_storage_type(const tree_t *self);
size_t tree_get_storage_size(const tree_t *self);

///
/// closure + function queries
///

const tree_t *tree_fn_get_return(const tree_t *self);
vector_t *tree_fn_get_params(const tree_t *self);
arity_t tree_fn_get_arity(const tree_t *self);

///
/// type queries
///

tree_t *tree_ty_get_field(const tree_t *self, const char *name);
tree_t *tree_ty_get_case(const tree_t *self, const char *name);
bool tree_ty_is_address(const tree_t *type);
quals_t tree_ty_get_quals(const tree_t *self);

/**
 * @brief get the type of a type after it has been loaded
 *
 * @note pointer -> type, reference -> type, etc.
 * @param self the type to load from
 * @return const tree_t* the loaded type
 */
const tree_t *tree_ty_load_type(const tree_t *self);
