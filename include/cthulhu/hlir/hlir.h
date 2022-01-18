#pragma once

#include "cthulhu/ast/ast.h"
#include "cthulhu/ast/ops.h"

typedef enum {
    HLIR_UNARY,
    HLIR_BINARY
} hlir_type_t;

typedef struct hlir_t {
    hlir_type_t type;
    node_t *node;

    union {
        struct {
            binary_t binary;
            struct hlir_t *lhs;
            struct hlir_t *rhs;
        };

        struct {
            unary_t unary;
            struct hlir_t *operand;
        };
    };
} hlir_t;
