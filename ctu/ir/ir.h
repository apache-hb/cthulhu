#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    OP_RETURN, /* return from function */
    OP_EMPTY, /* optimized out */
    OP_BINARY, /* binary operation */
    OP_UNARY, /* unary operation */
    OP_VALUE, /* either a copy or an immediate */

    OP_BLOCK, /* start of a basic block */
    OP_JUMP /* conditional jump */
} opcode_t;

typedef size_t vreg_t;

typedef struct {
    enum { VREG, IMM, NAME, NONE } kind;

    union {
        vreg_t vreg;
        uint64_t imm;
        const char *name;
    };
} operand_t;

typedef struct {
    opcode_t opcode;

    type_t *typeof;

    union {
        /* OP_RETURN, OP_VALUE */
        operand_t value;

        /* OP_UNARY */
        struct {
            unary_t unary;
            operand_t expr;
        };

        /* OP_BINARY */
        struct {
            binary_t binary;
            operand_t lhs;
            operand_t rhs;
        };
    };
} step_t;

/**
 * a single function
 */
typedef struct {
    step_t *steps;
    size_t len, size;
} flow_t;

/**
 * a compilation unit
 */
typedef struct {
    flow_t *flows;
    size_t len, size;
} module_t;

module_t compile_module(nodes_t *nodes);
