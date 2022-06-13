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

    union {
        struct {
            map_t *config;
        };
    };
} ast_t;

ast_t *ast_root(map_t *config);
