#pragma once

#include "cthulhu/ast/scan.h"
#include "cthulhu/ast/ast.h"

#include "std/vector.h"
#include "std/map.h"

typedef enum
{
    AST_GRAMMAR,

    AST_MAP,
    AST_VECTOR,
    AST_STRING,
    AST_IDENT,
    AST_PAIR,

    AST_TOTAL
} ast_kind_t;

typedef struct ast_t
{
    ast_kind_t kind;
    node_t node;

    union {
        map_t *map;

        vector_t *vector;

        char *string;

        struct
        {
            char *key;
            struct ast_t *value;
        };

        struct
        {
            map_t *config;
            map_t *lexer;
        };
    };
} ast_t;

ast_t *ast_grammar(scan_t *scan, where_t where, map_t *config, map_t *lexer);

map_t *build_map(scan_t *scan, where_t where, vector_t *entries);
ast_t *ast_vector(scan_t *scan, where_t where, vector_t *vector);
ast_t *ast_pair(scan_t *scan, where_t where, char *key, ast_t *value);
ast_t *ast_string(scan_t *scan, where_t where, char *string);
ast_t *ast_ident(scan_t *scan, where_t where, char *ident);

#define GENLTYPE where_t
