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
const attribs_t *tree_get_attrib(const tree_t *self);
const tree_t *tree_follow_type(const tree_t *self);

bool tree_is(const tree_t *self, tree_kind_t kind);

bool tree_has_quals(const tree_t *self, quals_t quals);
bool tree_has_vis(const tree_t *self, tree_visible_t visibility);
