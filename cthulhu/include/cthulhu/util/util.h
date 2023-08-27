#pragma once

#include <stdbool.h>

typedef struct tree_t tree_t;

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name);

bool util_types_equal(const tree_t *lhs, const tree_t *rhs);
