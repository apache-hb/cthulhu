#pragma once

typedef struct tree_t tree_t;

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name);
