#pragma once

typedef struct tree_t tree_t;

void ctu_resolve_function(tree_t *sema, tree_t *self, void *user);
void ctu_resolve_function_type(tree_t *sema, tree_t *self, void *user);
