#pragma once

#include "scan/node.h"

#include <gmp.h>

typedef struct map_t map_t;
typedef struct vector_t vector_t;

#define GENLTYPE where_t

typedef enum {
    AST_ROOT,

    AST_CONFIG,
    AST_TOKENS,

    AST_STRING,
    AST_IDENT,
    AST_DIGIT
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

        const char *str;
        mpz_t digit;
    };
} ast_t;

typedef struct {
    const char *key;
    ast_t *value;
} pair_t;

ast_t *ast_root(scan_t scan, where_t where, ast_t *config);

ast_t *ast_config(scan_t scan, where_t where, map_t *fields);
ast_t *ast_tokens(scan_t scan, where_t where, map_t *fields);

map_t *collect_map(scan_t scan, vector_t *fields);
pair_t *pair_new(const char *key, struct ast_t *ast);

ast_t *ast_string(scan_t scan, where_t where, const char *str);
ast_t *ast_ident(scan_t scan, where_t where, const char *str);
ast_t *ast_digit(scan_t scan, where_t where, mpz_t digit);
