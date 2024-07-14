// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

typedef struct tree_t tree_t;
typedef struct node_t node_t;

const tree_t *ctu_get_default_value(const node_t *node, const tree_t *type);
