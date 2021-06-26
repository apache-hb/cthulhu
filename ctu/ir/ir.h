#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    OP_EMPTY, /* optimized out */

    OP_RETURN, /* return from function */
    OP_BINARY, /* binary operation */
    OP_UNARY, /* unary operation */
    OP_CALL, /* calling a function */
    OP_VALUE, /* either a copy or an immediate */
    OP_CONVERT, /* convert a value to another type */

    OP_RESERVE, /* reserve space for type */
    OP_LOAD, /* load from memory */
    OP_STORE, /* store to memory */

    OP_BLOCK, /* start of a basic block */
    OP_BRANCH /* conditional jump */
} opcode_t;

typedef size_t vreg_t;

typedef struct {
    enum { IMM_BOOL, IMM_INT } kind;

    union {
        bool imm_bool;
        int64_t imm_int;
    };
} imm_t;

typedef struct {
    enum { 
        ARG, /* an argument passed to this function */
        VREG, /* a virtual register in the current flow */
        BLOCK, /* an address of a basic block in the current flow */
        
        IMM, /* an immediate value */
        
        FUNC, /* another flow in the current unit */
        GLOBAL, /* a global variable */
        
        NONE /* nothing */
    } kind;

    union {
        /* virtual register in current flow */
        vreg_t vreg;

        /* argument in current flow */
        vreg_t arg;

        /* basic block in current flow */
        size_t label;

        /* global function index */
        size_t func;

        /* global variable index */
        size_t var;

        /* immediate value */
        imm_t imm;
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

        /* OP_RESERVE */
        operand_t size;

        /* OP_STORE, OP_LOAD */
        struct {
            operand_t src;
            operand_t dst;
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

    vreg_t *locals;
    size_t nlocals;

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
    size_t nflows;
} module_t;

size_t num_flows(module_t *mod);

module_t compile_module(nodes_t *nodes);
