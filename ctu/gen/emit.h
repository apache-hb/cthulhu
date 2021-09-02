#pragma once

#include "ctu/util/util.h"
#include "ctu/type/type.h"
#include "ctu/lir/lir.h"

typedef struct {
    type_t *type;
} value_t;

typedef enum {
    IMM,
    VREG,
    LABEL,
    EMPTY
} optype_t;

typedef struct {
    optype_t kind;

    union {
        size_t vreg;
        size_t label;
    };
} operand_t;

typedef enum {
    OP_UNARY
} opcode_t;

typedef struct {
    opcode_t opcode;
    node_t *node;
    type_t *type;

    union {
        struct {
            unary_t unary;
            operand_t operand;
        };
    };
} step_t;

typedef struct {
    const char *name;
    value_t *value;

    size_t len;
    size_t size;
    step_t *steps;
} flow_t;

typedef struct {
    const char *name;

    vector_t *flows;
} module_t;

module_t *module_build(lir_t *root);
