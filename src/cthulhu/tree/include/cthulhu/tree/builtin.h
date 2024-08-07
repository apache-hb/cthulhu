#pragma once

#include <ctu_tree_api.h>

typedef struct tree_t tree_t;
typedef struct node_t node_t;

CT_BEGIN_API

CT_TREE_API tree_t *tree_builtin_sizeof(const node_t *node, const tree_t *type, const tree_t *size_type);
CT_TREE_API tree_t *tree_builtin_alignof(const node_t *node, const tree_t *type, const tree_t *align_type);
CT_TREE_API tree_t *tree_builtin_offsetof(const node_t *node, const tree_t *type, const tree_t *field, const tree_t *offset_type);

CT_END_API
