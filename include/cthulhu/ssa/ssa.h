#pragma once

#include "cthulhu/util/report.h"
#include "cthulhu/hlir/hlir.h"

typedef enum {
    OP_VREG, // a virtual register from within the block
    OP_IMM, // an immediate value
    OP_ADDR, // an address of another block

    OP_EMPTY // nothing
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
    STEP_LOAD, // vreg = *src
    STEP_STORE, // *dst = src

    STEP_BRANCH, // if (cond) goto target else goto other
    STEP_JUMP, // goto target

    STEP_RETURN // ret result
} opcode_t;

typedef struct {
    opcode_t op;

    union {
        // STEP_LOAD uses only `src`
        // STEP_STORE uses both `src` and `dst`
        struct {
            operand_t dst;
            operand_t src;
        };

        // STEP_JUMP only uses `target`
        // STEP_BRANCH uses all 3
        struct {
            operand_t cond;
            operand_t target;
            operand_t other;
        };

        // STEP_RETURN return value
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
