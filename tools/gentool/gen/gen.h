#pragma once

#include "scan/node.h"

#include "std/map.h"

#define GENLTYPE where_t

typedef enum {
    AST_ROOT,

    AST_CONFIG,
    AST_TOKENS,
    AST_GRAMMAR
} ast_kind_t;

typedef struct ast_t {
    ast_kind_t kind;
    node_t node;

    union {
        struct {
            struct ast_t *config;
            struct ast_t *tokens;
            struct ast_t *grammar;
        };

        map_t *fields;
    };
} ast_t;

ast_t *ast_root(node_t node, ast_t *config);

ast_t *ast_config(node_t node);
ast_t *ast_tokens(node_t node);
ast_t *ast_grammar(node_t node);
