#pragma once

#include "ast.h"

typedef struct h2_t h2_t;
typedef struct suffix_t suffix_t;
typedef struct ast_t ast_t;

typedef h2_t *(*apply_suffix_t)(h2_t *sema, ast_t *ast, suffix_t *suffix);

typedef struct suffix_t
{
    astof_t expected;
    apply_suffix_t apply;
    void *data;
} suffix_t;

void add_builtin_suffixes(h2_t *sema);

h2_t *apply_suffix(h2_t *sema, ast_t *ast, suffix_t *suffix);
