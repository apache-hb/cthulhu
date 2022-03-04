#pragma once

#include "cthulhu/util/report.h"
#include "cthulhu/hlir/hlir.h"

typedef enum {
    OP_VREG,
    OP_IMM,
    OP_ADDR
} optype_t;

typedef struct {
    optype_t op;

    union {
        size_t vreg;
        const hlir_t *imm;
        struct block_t *addr;
    };
} operand_t;

typedef enum {
    STEP_LOAD,
    STEP_STORE,
    STEP_RETURN
} opcode_t;

typedef struct {
    opcode_t op;

    union {
        operand_t src;
        operand_t dst;
        operand_t result;
    };
} step_t;

typedef struct block_t {
    const char *name;

    size_t length;
    size_t capacity;
    step_t *steps;
} block_t;

typedef struct {
    const char *name;
    map_t *blocks;
    map_t *globals;
} module_t;

module_t *build_ssa(reports_t *reports, const hlir_t *root);
