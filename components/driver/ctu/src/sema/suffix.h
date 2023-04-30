#pragma once

#include "ast.h"

typedef struct hlir_t hlir_t;
typedef struct suffix_t suffix_t;
typedef struct ast_t ast_t;
typedef struct sema_t sema_t;

typedef hlir_t *(*apply_suffix_t)(sema_t *sema, ast_t *ast, suffix_t *suffix);

typedef struct suffix_t
{
    astof_t expected;
    apply_suffix_t apply;
    void *data;
} suffix_t;

void add_builtin_suffixes(sema_t *sema);

hlir_t *apply_suffix(sema_t *sema, ast_t *ast, suffix_t *suffix);
