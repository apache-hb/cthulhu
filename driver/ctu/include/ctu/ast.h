#pragma once

#include "scan/scan.h"
#include "scan/node.h"

typedef struct ast_t ast_t;
typedef struct vector_t vector_t;

typedef enum ast_kind_t {
    eAstModule
} ast_kind_t;

typedef struct ast_t {
    ast_kind_t kind;
    node_t *node;

    union {
        struct {
            vector_t *modspec;
        };
    };
} ast_t;

ast_t *ast_module(scan_t *scan, where_t where, vector_t *modspec);
