#pragma once

#include <stddef.h>
#include <stdint.h>

#include "cthulhu/front/ast.h"
#include "cthulhu/util/util.h"

/**
 * our IR looks alot like llvms
 * they mostly got it right so no need to reinvent the wheel.
 */

typedef enum {
    /* %vreg = |op| */
    OP_ABS, 
    
    /* %vreg = op * -1 */
    OP_NEG,

    /* %vreg = op */
    OP_VALUE,

    /* %vreg = (type)op */
    OP_CAST,

    /* %vreg = call op */
    OP_CALL,

    /* %vreg = select cond true false */
    OP_SELECT,

    /* branch lhs when cond else rhs */
    OP_BRANCH, 
 
    /* jmp block */
    OP_JUMP,

    /* %vreg = [ lhs, rhs ] */
    OP_PHI, 

    /* block: */
    OP_BLOCK,

    /* ret op */
    OP_RET,

    /* %vreg = lhs + rhs */
    OP_ADD, 
    
    /* %vreg = lhs - rhs */
    OP_SUB, 
    
    /* %vreg = lhs / rhs */
    OP_DIV, 
    
    /* %vreg = lhs * rhs */
    OP_MUL, 
    
    /* %vreg = lhs % rhs */
    OP_REM,

    /* optimised away */
    OP_EMPTY
} optype_t;

typedef size_t vreg_t;

typedef enum {
    VREG, /* virtual register */
    IMM_L, /* long immediate value */
    IMM_I, /* integer immediate value */
    BIMM, /* immediate bool */
    NAME, /* a function name */
    NONE /* void value */
} opkind_t;

typedef struct {
    opkind_t kind;
    union {
        /* VREG */
        vreg_t vreg;
        /* IMM_L */
        long long imm_l;
        /* IMM_I */
        int imm_i;
        /* BIMM */
        bool bimm;
        /* NAME */
        const char *name;
    };
    /* the block this operand is live in */
    size_t block;
} operand_t;

typedef struct {
    size_t block;
    operand_t val;
} branch_t;

typedef struct {
    optype_t kind;

    union {
        /* OP_ABS, OP_NEG, OP_VALUE, OP_RET, OP_CALL */
        struct {
            operand_t expr;
            /* OP_CAST */
            opkind_t cast;
        };

        /* OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_REM */
        struct {
            /* OP_SELECT, OP_BRANCH */
            operand_t cond;

            /* OP_PHI */
            operand_t lhs; /* true leaf */
            operand_t rhs; /* false leaf */
        };

        /* OP_JUMP */
        size_t label;
    };
} op_t;

/* the control flow of a single function */
typedef struct {
    const char *name;

    /* return type */
    opkind_t result;

    op_t *ops;
    size_t size;
    size_t len;
} flow_t;

/* a compilation unit */
typedef struct {
    const char *name;
    
    flow_t *flows;
    size_t size;
    size_t len;
} unit_t;

/**
 * convert a compiled file into a compilation unit
 */
unit_t transform_ast(const char *name, nodes_t *node);
