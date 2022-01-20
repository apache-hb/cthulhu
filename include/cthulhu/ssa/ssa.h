#pragma once

#include "cthulhu/hlir/hlir.h"

#include "value.h"

typedef enum {
    /* operations that store to a register */
    OP_VALUE,

    /* operations that do not store to a register */
    OP_RETURN,
    OP_ASSIGN,
} step_type_t;

typedef enum {
    step_type_t kind;
    node_t *node;
} step_t;

typedef struct {
    const char *name;

    size_t length;
    step_t *steps;
} block_t;

typedef struct {
    vector_t *globals;
    vector_t *functions;
} module_t;
