#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    OP_RETURN, /* return from function */
    OP_EMPTY, /* optimized out */
    OP_BINARY, /* binary operation */
    OP_UNARY, /* unary operation */
    OP_CALL, /* calling a function */
    OP_VALUE, /* either a copy or an immediate */
    OP_CONVERT, /* convert a value to another type */

    OP_BLOCK, /* start of a basic block */
    OP_BRANCH /* conditional jump */
} opcode_t;

typedef size_t vreg_t;

typedef struct {
    enum { 
        ARG, /* an argument passed to this function */
        VREG, /* a virtual register in the current flow */
        BLOCK, /* an address of a basic block in the current flow */
        IMM, /* an immediate value */
        NAME, /* an external function */
        FUNC, /* another flow in the current unit */
        NONE /* nothing */
    } kind;

    union {
        vreg_t vreg;
        vreg_t arg;
        size_t label;
        size_t func;
        int64_t imm;
        const char *name;
    };
} operand_t;

typedef struct {
    opcode_t opcode;

    type_t *type;

    union {
        /* OP_RETURN, OP_VALUE */
        struct {
            operand_t value;

            operand_t *args;
            size_t len;
        };

        /* OP_BRANCH */
        struct {
            /**
             * if cond then 
             *  goto block 
             * else 
             *  goto other
             * end
             */
            operand_t cond;
            operand_t block;
            operand_t other;
        };

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

typedef struct {
    const char *name;
    type_t *type;
} arg_t;

/**
 * a single function
 */
typedef struct {
    /* the name of the function */
    const char *name;

    arg_t *args;
    size_t nargs;

    step_t *steps;
    size_t len, size;

    /* the return type */
    type_t *result;

    /* the parent module this flow is contained in */
    struct module_t *mod;
} flow_t;

step_t *step_at(flow_t *flow, size_t idx);

/**
 * a compilation unit
 */
typedef struct module_t {
    flow_t *flows;
    size_t len;
} module_t;

module_t compile_module(nodes_t *nodes);
