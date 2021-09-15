#pragma once

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"

#include "value.h"

typedef enum {
    IMM,
    VREG,
    LABEL,
    ADDRESS,
    EMPTY
} optype_t;

typedef struct {
    optype_t kind;

    union {
        value_t *imm;
        size_t vreg;
        size_t label;
        struct block_t *block;
    };
} operand_t;

typedef enum {
    OP_EMPTY,
    OP_UNARY,
    OP_BINARY,
    OP_CALL,
    OP_RETURN,

    OP_LOAD,
    OP_STORE,

    OP_BRANCH,
    OP_JMP,
    OP_BLOCK
} opcode_t;

typedef struct {
    opcode_t opcode;
    node_t *node;
    const type_t *type;

    union {
        struct {
            operand_t func;
            operand_t *args;
            size_t len;
        };

        struct {
            operand_t src;
            operand_t dst;
            operand_t offset;
        };

        struct {
            operand_t cond;
            operand_t label;
            operand_t other;
        };
        
        struct {
            unary_t unary;
            operand_t operand;
        };

        struct {
            binary_t binary;
            operand_t lhs;
            operand_t rhs;
        };
    };
} step_t;

typedef struct block_t {
    const char *name;

    /* the return type of this */
    const type_t *result;
    const value_t *value;

    size_t len;
    size_t size;
    step_t *steps;
} block_t;

typedef struct {
    const char *name;

    vector_t *vars;
    vector_t *funcs;
} module_t;

module_t *module_build(reports_t *reports, lir_t *root);
void module_print(FILE *out, module_t *mod);
