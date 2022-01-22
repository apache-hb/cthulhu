#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report.h"

#include "cthulhu/data/value.h"

typedef enum {
    OPERAND_EMPTY,
    OPERAND_VREG,
    OPERAND_VALUE,
    OPERAND_BLOCK,
    OPERAND_LABEL
} operand_type_t;

typedef struct {
    operand_type_t type;

    union {
        size_t vreg;
        size_t label;
        value_t *value;
        struct block_t *block;
    };
} operand_t;

typedef enum {
    OP_LOAD, // vreg = *addr
    OP_CALL, // vreg = func(args...)
    OP_BINARY, // vreg = lhs op rhs
    OP_LOCAL, // allocate space on the stack

    OP_LABEL, // label:
    OP_BRANCH, // if (cond) goto true else goto false
    OP_COMPARE, // vreg =  (lhs op rhs)
    OP_JMP, // goto label
    OP_STORE, // *addr = vreg
    OP_RETURN // return vreg
} step_type_t;

typedef struct {
    step_type_t type;
    const node_t *node;

    union {
        operand_t value;

        struct {
            operand_t call;
            operand_t *operands;
            size_t total;
        };

        struct {
            operand_t lhs;
            operand_t rhs;
            union {
                binary_t binary;
                compare_t compare;
            };
        };

        struct {
            operand_t dst;
            operand_t src;
        };

        struct {
            operand_t cond;
            operand_t then;
            operand_t other;
        };
    };
} step_t;

typedef struct block_t {
    const char *name;

    size_t length;
    step_t *steps;
} block_t;

typedef struct {
    const char *name;
    const scan_t *source;

    vector_t *globals;
    vector_t *functions;
} module_t;

typedef struct {
    reports_t *reports;

    map_t *blocks;

    step_t *steps;
    size_t length;
    size_t used;
} ssa_t;

block_t *build_global(ssa_t *ssa, const hlir_t *hlir);
module_t *build_module(ssa_t *ssa, const hlir_t *hlir);

ssa_t *build_ssa(reports_t *reports);
