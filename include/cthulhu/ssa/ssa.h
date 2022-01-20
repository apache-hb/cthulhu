#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report.h"

typedef enum {
    OPERAND_EMPTY,
    OPERAND_VREG,
    OPERAND_VALUE,
    OPERAND_BLOCK
} operand_type_t;

typedef struct {
    operand_type_t type;

    union {
        size_t vreg;
        mpz_t value;
        struct block_t *block;
    };
} operand_t;

typedef enum {
    OP_LOAD,
    OP_BINARY,

    OP_RETURN
} step_type_t;

typedef struct {
    step_type_t type;
    const node_t *node;

    union {
        operand_t value;

        struct {
            operand_t lhs;
            operand_t rhs;
            binary_t binary;
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
